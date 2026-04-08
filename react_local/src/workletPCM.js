class WorkletPCM{
	constructor() {
		this.recorderData = null;
	}
	static create() {
		return new WorkletPCM();
	}
	async startRecording(instream) {
		const audioContext = new AudioContext({ sampleRate: 16000 });
		await audioContext.audioWorklet.addModule('recorder-processor.js');

		const source = audioContext.createMediaStreamSource(instream);

		const recorderNode = new AudioWorkletNode(audioContext, 'recorder-processor');
		source.connect(recorderNode);
		// 如果不需要即時播放，也可以不 connect destination
		//recorderNode.connect(audioContext.destination);

		// 存放 PCM
		const recordedPCM = [];

		recorderNode.port.onmessage = (event) => {
			if (!event.data) return;
			recordedPCM.push(new Float32Array(event.data));
		};

		return { audioContext, recorderNode, recordedPCM, instream };
	}
	stopRecording({ audioContext, recorderNode, recordedPCM, stream }) {
		recorderNode.disconnect();
		audioContext.close();
		//stream.getTracks().forEach(track => track.stop());

		// 合併所有 chunk
		const totalLength = recordedPCM.reduce((sum, chunk) => sum + chunk.length, 0);
		const interleaved = new Float32Array(totalLength);
		let offset = 0;
		for (const chunk of recordedPCM) {
			interleaved.set(chunk, offset);
			offset += chunk.length;
		}

		// 生成 WAV Blob
		const wavBlob = this.float32ToWav(interleaved, audioContext.sampleRate);
		//console.log(wavBlob)
		return wavBlob;
	}
	float32ToWav(float32Array, sampleRate) {
		const bufferLength = float32Array.length;
		const wavBuffer = new ArrayBuffer(44 + bufferLength * 2);
		const view = new DataView(wavBuffer);

		/* RIFF identifier */
		this.writeString(view, 0, 'RIFF');
		/* file length */
		view.setUint32(4, 36 + bufferLength * 2, true);
		/* RIFF type */
		this.writeString(view, 8, 'WAVE');
		/* format chunk identifier */
		this.writeString(view, 12, 'fmt ');
		/* format chunk length */
		view.setUint32(16, 16, true);
		/* sample format (1 = PCM) */
		view.setUint16(20, 1, true);
		/* channel count */
		view.setUint16(22, 1, true);
		/* sample rate */
		view.setUint32(24, sampleRate, true);
		/* byte rate (sampleRate * blockAlign) */
		view.setUint32(28, sampleRate * 2, true);
		/* block align (channelCount * bytesPerSample) */
		view.setUint16(32, 2, true);
		/* bits per sample */
		view.setUint16(34, 16, true);
		/* data chunk identifier */
		this.writeString(view, 36, 'data');
		/* data chunk length */
		view.setUint32(40, bufferLength * 2, true);

		// PCM 轉 16-bit
		let offset = 44;
		for (let i = 0; i < bufferLength; i++, offset += 2) {
			let s = Math.max(-1, Math.min(1, float32Array[i]));
			view.setInt16(offset, s < 0 ? s * 0x8000 : s * 0x7fff, true);
		}

		return new Blob([view], { type: 'audio/wav' });
	}

	writeString(view, offset, string) {
		for (let i = 0; i < string.length; i++) {
			view.setUint8(offset + i, string.charCodeAt(i));
		}
	}

	

	async StartWorklet(instream) {
		this.recorderData = await this.startRecording(instream);
	}
}


export default WorkletPCM;