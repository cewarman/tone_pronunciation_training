class DrawPitch {
	constructor() {
		this.ws = null;
		this.pitch_url = null;
	}
	static create() {
		return new DrawPitch();
	}
	pitchdraw({ ws, url, pd }) {
		let sampleperMS = ws.decodedData.sampleRate * 0.01
		//const pitch_data_path = url.substring(0, url.indexOf('wav')) + 'txt'

		const minPitch = 4.3
		const maxPitch = 6.69   // 依你的 pitch 範圍調整

		const pitch_data_path = url

		let canvas = ""
		const oldCanvas = ws.renderer.getWrapper().querySelector('.pitch')
		if (!oldCanvas) {
			canvas = document.createElement('canvas')
		}
		else {
			canvas = oldCanvas
		}
		
		canvas.style.top = '-70px'
		canvas.className = 'pitch'
		const ctx = canvas.getContext('2d')

		const dpr = window.devicePixelRatio || 1  // 裝置像素比
		const height = 250
		const pitchColor = '#000000ff'
		const poffset = height * 4.2
		const pratio = height/1.5


		// 設定 canvas 實際像素尺寸
		canvas.width = Math.min(ws.decodedData.length * dpr, 32767)
		canvas.height = height * dpr
		sampleperMS = (canvas.width / (ws.decodedData.length * dpr)) * ws.decodedData.sampleRate * 0.01

		// 設定顯示尺寸（CSS 大小不變）

		canvas.style.width = '100%'
		canvas.style.height = `${height}px`
		const xscale = canvas.width / ws.renderer.lastContainerWidth / dpr
		let pitch_data = 0
		ctx.scale(dpr * xscale, dpr) // 讓繪圖座標自動縮放
		if(pd==''){
			fetch(pitch_data_path)
				.then((response) => response.text())
				.then((text) => {
					pitch_data = text.split('\n').map(Number)
					canvas.pitch_data = pitch_data
					for (let i = 0; i < pitch_data.length; i++) {
						//const y = Math.round(poffset - pitch_data[i] * pratio)
						const y =height - ((pitch_data[i] - minPitch) / (maxPitch - minPitch)) * height
						//const y =height - ((pitch_data[i] - minPitch) / (maxPitch - minPitch)) * height
						ctx.fillStyle = pitchColor
						//ctx.fillRect(i * sampleperMS / xscale, y, xlabelsize, ylabelsize) // 不再乘 dpr
						ctx.beginPath()
						ctx.arc(i * sampleperMS / xscale, y, 1, 0, 2 * Math.PI, false)
						ctx.fill()
					}
					const wrapper = ws.renderer.getWrapper()
					//wrapper.style.position = 'relative'
					//canvas.style.zIndex = '2'
					wrapper.appendChild(canvas)
					ws.renderer.getWrapper().appendChild(canvas)
				})
		}
		else{
			pitch_data = pd.split(' ').map(Number)
			//console.log(pitch_data)
			canvas.pitch_data = pitch_data
			//console.log(canvas.pitch_data)
			for (let i = 0; i < pitch_data.length; i++) {
				//const y = Math.round(poffset - pitch_data[i] * pratio)
				const y =height - ((pitch_data[i] - minPitch) / (maxPitch - minPitch)) * height
				ctx.fillStyle = pitchColor
				//ctx.fillRect(i * sampleperMS / xscale, y, xlabelsize, ylabelsize) // 不再乘 dpr
				ctx.beginPath()
				ctx.arc(i * sampleperMS / xscale, y, 1, 0, 2 * Math.PI, false)
				ctx.fill()
			}
			const wrapper = ws.renderer.getWrapper()
			wrapper.style.position = 'relative'
			canvas.style.zIndex = '2'
			wrapper.querySelectorAll('.pitch').forEach(c => c.remove())
			wrapper.appendChild(canvas)
		}
		
	}
	zoompitchdraw({ ws }) {
		const minPitch = 4.3
		const maxPitch = 6.69 

		let sampleperMS = ws.decodedData.sampleRate * 0.01
		const oldCanvas = ws.renderer.getWrapper().querySelector('.pitch')
		if (!oldCanvas) {
			return
		}
		const dpr = window.devicePixelRatio || 1
		sampleperMS = (oldCanvas.width / (ws.decodedData.length * dpr)) * ws.decodedData.sampleRate * 0.01
		const ctx = oldCanvas.getContext('2d')
		const pitchColor = '#000000ff'
		const height = 250
		const poffset = height * 4.2
		const pratio = height / 1.5
		const xscale = oldCanvas.width / ws.renderer.lastContainerWidth / dpr
		const roominscale = (ws.renderer.getWrapper().clientWidth / ws.renderer.lastContainerWidth)
		ctx.setTransform(1, 0, 0, 1, 0, 0)
		ctx.scale(dpr * xscale / roominscale, dpr)

		let pitch_data = oldCanvas.pitch_data
		//console.log(pitch_data)
		ctx.clearRect(0, 0, oldCanvas.width, oldCanvas.height);
		for (let i = 0; i <= pitch_data.length - 1; i++) {
			//const y = Math.round(poffset - (pitch_data[i]) * pratio)
			const y =height - ((pitch_data[i] - minPitch) / (maxPitch - minPitch)) * height
			ctx.fillStyle = pitchColor
			//ctx.fillRect(i*sampleperMS / xscale, y, xlabelsize, ylabelsize)
			ctx.beginPath()
			ctx.arc((i * sampleperMS / xscale) * roominscale, y, 1, 0, 2 * Math.PI, false)
			ctx.fill()
		}

		ws.resizescale = 1
	}
}
export default DrawPitch;