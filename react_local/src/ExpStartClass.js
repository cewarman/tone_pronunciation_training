class expstartclass {
	constructor(PI, A, G, M, S, T, EG, setDisplayImage, setwavePlayer_url, setshowed_text) {
		this.PI = PI;
		this.A = A;
		this.G = G;
		this.M = M;
		this.S = S;
		this.T = T;
		this.EG = EG;
		this.setDisplayImage = setDisplayImage;
		this.setwavePlayer_url = setwavePlayer_url;
		this.setshowed_text = setshowed_text;
		this.exp_file_list = ['mono_example_fan1', 'mono_example_wu2', 'mono_example_kou3', 'mono_example_chu4']
	}
	exp_start({ PI, A, G, M, S, T, EG, setDisplayImage, setwavePlayer_url, setshowed_text }) {
		console.log(PI, A, G, M, S, T, EG)
		const imageMap = {
			PC: "/exp_data/five_tone_value.png",
			PPC: "/exp_data/standard_tone.png",
		}
		setDisplayImage(imageMap[EG])

		//document.body.appendChild(WavePlayer("./exp_data/mono_example_fan1.wav"))
		//document.querySelector('.App').appendChild(WavePlayer("./exp_data/mono_example_fan1.wav"))
		setshowed_text("翻")
		setwavePlayer_url("exp_data/" + exp_file_list[0] + ".wav")

	}
	handlenextwav() {

	}
}
export default expstartclass;