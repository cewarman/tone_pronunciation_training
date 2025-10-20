class RecorderProcessor extends AudioWorkletProcessor {
  constructor() {
    super();
  }

  process(inputs, outputs, parameters) {
    const input = inputs[0]; // 第一個 channel
    if (input.length > 0) {
      const channelData = input[0]; // Float32Array
      // 傳到 main thread
      this.port.postMessage(channelData);
    }
    return true; // 繼續運作
  }
}

registerProcessor('recorder-processor', RecorderProcessor);