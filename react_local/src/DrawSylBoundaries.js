class DrawSylBoundaries {
	constructor() {
		this.ws = null;
		this.url = null;
	}
	static create() {
		return new DrawSylBoundaries();
	}
	drawsylboundaries({ ws, url, sd}) {
		let sylnum = 0
		let rp = null
		//console.log(ws,ws.plugins)
		for (let i = 0; i <= ws.plugins.length - 1; i++) {
			if (ws.plugins[i].id === 'regionplugin') {
				rp = ws.plugins[i]
			}
		}
		if (rp == null) {
			console.warn('⚠️ region plugin not found, skip drawsylboundaries.')
			return
		}
		const syl_data_path = url
		const regions = rp

		if(sd==''){
			fetch(syl_data_path).then((response) => response.text()).then((text) => {
				let syl_data = text.split('\n')
				for (let i = 0; i <= syl_data.length - 2; i++) {

					const mark = syl_data[i].split(/\s+/)
					//console.log(mark[1], mark[2])
					regions.addRegion({
						start: mark[1],
						content: mark[2],
						color: "rgba(0, 0, 0, 0.3)",
						drag: true,
					})
					sylnum = sylnum + 1
					ws.sylnum = sylnum
				}
			})
		}
		else{
			let syl_data = sd.split('\n')
			for (let i = 0; i <= syl_data.length - 1; i++) {

				const mark = syl_data[i].split(/\s+/)
				//console.log('mark1', mark[1], 'mark2', mark[2])
				regions.addRegion({
					start: mark[1],
					content: mark[2],
					color: "rgba(0, 0, 0, 0.3)",
					drag: true,
				})
				sylnum = sylnum + 1
				ws.sylnum = sylnum
			}
		}
		

	}
	clearlastregion({ ws }) {
		let rp = null
		for (let i = 0; i <= ws.plugins.length - 1; i++) {
			if (ws.plugins[i].id === 'regionplugin') {
				rp = ws.plugins[i]
			}
		}
		if (rp.regions.length > ws.sylnum) {
			rp.regions[ws.sylnum].remove()
		}
	}

}
export default DrawSylBoundaries;