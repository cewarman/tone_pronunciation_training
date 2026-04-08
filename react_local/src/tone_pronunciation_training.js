import { useEffect, useRef, useState } from "react"
import WaveSurfer from "wavesurfer.js"
import TimelinePlugin from "wavesurfer.js/dist/plugins/timeline.esm.js"
import Minimap from 'wavesurfer.js/dist/plugins/minimap.esm.js'
import Hover from 'wavesurfer.js/dist/plugins/hover.esm.js'
import RegionsPlugin from 'wavesurfer.js/dist/plugins/regions.esm.js'
import RecordPlugin from '../node_modules/wavesurfer.js/dist/plugins/record2.js'
import WorkletPCM from "./workletPCM.js"
import DrawPitch from "./DrawPitch.js"
import DrawSylBoundaries from "./DrawSylBoundaries.js"

//console.log(WorkletPCM)
//console.log(DrawPitch)
//console.log(DrawSylBoundaries)
//indexedDB.deleteDatabase("tpt-db")
let wlp = WorkletPCM.create();
let dp = DrawPitch.create();
let dsb = DrawSylBoundaries.create();
let GOPdist={}
fetch("GOPdist.json")
  .then(res => res.json())
  .then(data => {
    //console.log(data['a-1']);
    GOPdist=data
  })

//var websocket = new WebSocket("ws://120.126.151.132:9999");
//need to run: cloudflared tunnel --url http://localhost:9999 --protocol http2
const wsUrl = "wss://favourite-speeds-ratings-annually.trycloudflare.com"
var websocket = null//new WebSocket(wsurl);
let reconnectInterval = 3000; // 3秒後重連

function connectWS() {
  websocket = new WebSocket(wsUrl);

  websocket.onopen = () => {
    console.log("WebSocket 已連線");
  };

  //websocket.onmessage = (event) => {
  //  console.log("收到訊息:", event.data);
  //};

  websocket.onclose = (e) => {
    console.log(`WebSocket 斷線 (Code: ${e.code})，${reconnectInterval / 1000} 秒後嘗試重連...`);

    // 清除舊的引用，並延遲重連
    websocket = null;
    setTimeout(() => {
      connectWS();
    }, reconnectInterval);
  };

  websocket.onerror = (err) => {
    console.error("WebSocket 發生錯誤:", err);
    websocket.close(); // 觸發 onclose 進行重連
  };
}
connectWS()
async function get_BPT(wavBlob, wavid, context, group) {
  console.log("WebSocket connected");
  websocket.send(wavid)
  websocket.send(context)
  websocket.send(group)
  const arrayBuffer = await wavBlob.arrayBuffer();
  websocket.send(arrayBuffer);
  console.log("Sent audio data");
};
async function get_recompute_info(bdinfo) {
  websocket.send(bdinfo)
  console.log("Sent bdinfo");
};
async function send_profile(profile) {
  websocket.send(profile)
  console.log("Sent profile");
};
async function send_SE_info(SE_info) {
  websocket.send(SE_info)
  console.log("Sent SE");
};
////websocket.onopen = () => websocket.send("hello");
//websocket.onmessage = e => console.log("got:", e.data);

function openDB(callback) {
  const request = indexedDB.open("tpt-db", 1)

  request.onupgradeneeded = () => {
    const db = request.result
    if (!db.objectStoreNames.contains("recordings")) {
      const store = db.createObjectStore("recordings", { keyPath: "id", autoIncrement: true })
      // id 自動增長
      store.createIndex("time", "time")  // optional, 用來排序
    }
    if (!db.objectStoreNames.contains("states")) {
      db.createObjectStore("states")
    }
  }

  request.onsuccess = () => {
    const db = request.result
    callback(db)
  }

  request.onerror = () => console.log("error", request.error)
}
function loadAllRecordings(callback) {
  openDB((db) => {
    const tx = db.transaction("recordings", "readonly")
    const store = tx.objectStore("recordings")
    const req = store.getAll()  // 取得所有錄音

    req.onsuccess = () => {
      // req.result 是一個 array，裡面包含每個 { blob, time, Fileidx, profile }
      callback(req.result)
    }

    req.onerror = () => console.log("load error", req.error)
  })
}
function Self_Evaluation({ Fileidx, reading_times, setNext_Enable, setRecord_Enable }) {
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [tempOption, setTempOption] = useState({});
  const [confidence, setConfidence] = useState("");
  //can you tell from the contours whether\n your tone pronumciations are correct? totally, most, some, few, not at all

  useEffect(() => {
    setTempOption({})
    setConfidence("")
  }, [Fileidx])

  const options = [["yu", "san", "tong"], ["ji", "tui", "tang"], ["men", "kou", "pang"], ["you", "yi", "ge"], ["wu", "chu", "le"], ["qian", "san", "le"], ["kuai", "san", "kai"],
  ["bu", "xiao", "xin"], ["yi", "yi", "bu", "she"], ["you", "chu", "xian", "le"], ["ci", "deng", "zhi", "yuan"], ["wu", "du", "you", "ou"], ["fang", "zai", "shi", "zhuo", "shang"],
  ["da", "fan", "zai", "yi", "fu", "shang"], ["ci", "deng", "zhi", "yuan", "dou", "bei", "qian", "san", "le"], ["wo", "ma", "ba", "fang", "ka", "fang", "zai", "shi", "zhuo", "shang"],
  ["ye", "ye", "dui", "zhe", "pian", "fu", "di", "yi", "yi", "bu", "she"], ["kuai", "san", "kai", "ta", "wu", "chu", "le", "she", "ji", "kai", "guan"],
  ["qian", "mian", "de", "ai", "fang", "men", "kou", "pang", "you", "yi", "ge", "yu", "san", "tong"],
  ["wu", "du", "you", "ou", "you", "chu", "xian", "le", "yi", "ge", "you", "xiu", "de", "ren", "cai"],
  ["ta", "mai", "de", "ji", "tui", "tang", "bu", "xiao", "xin", "da", "fan", "zai", "yi", "fu", "shang", "le"]];

  const handleSubmit = () => {

    if (Object.keys(tempOption).length === Object.keys(options[Fileidx - 20]).length && confidence) {
      setIsModalOpen(false);
      const sortedEntries = Object.entries(tempOption).sort((a, b) => Number(a[0].at(-1)) - Number(b[0].at(-1)))
      const sortedByValue = Object.fromEntries(sortedEntries)
      console.log(tempOption);
      console.log(confidence);
      setNext_Enable(true)
      setRecord_Enable(true)
      send_SE_info("self_evaluation|" + Fileidx + "|" + reading_times + "|" + sortedEntries + "|" + confidence)
    }
  };

  return (
    <div>
      {/* <h1>can you tell from the contours whether your tone pronunciations are correct?</h1> */}
      <button disabled={!(Fileidx >= 20 && reading_times > 0)} onClick={() => setIsModalOpen(true)}>Self Evaluation</button>

      {/* Modal */}
      {isModalOpen && (
        <div
          style={{
            position: "fixed",
            top: 0,
            left: 0,
            width: "100vw",
            height: "100vh",
            backgroundColor: "rgba(0,0,0,0.5)",
            display: "flex",
            justifyContent: "flex-start",
            alignItems: "flex-start",
            zIndex: 9999,
          }}
        >
          <div style={{ backgroundColor: "white", width: "80vw", padding: "20px", borderRadius: "8px", minWidth: "200px", }} >

            <div style={{ display: "flex", justifyContent: "space-between", alignItems: "flex-start" }}>
              <div>
                <h2 style={{ display: "flex", justifyContent: "flex-start" }}>Please rely on pitch contours to identify tones.</h2>
                <div style={{ display: "grid", gridAutoFlow: "column", gridTemplateRows: "repeat(7, 1fr)", rowGap: "0px", columnGap: "40px", }}>
                  {options[Fileidx - 20].map((word, idx) => (
                    <div key={`${word}-${idx}`} style={{ margin: "5px 0" }}>
                      <div>{word}:
                        {[1, 2, 3, 4, 5].map((tone) => (
                          <label key={tone} style={{ marginRight: "4px" }}>
                            <input type="radio" name={`word-${idx}`} value={tone} checked={tempOption[`${word}-${idx}`] === tone}
                              onChange={() => setTempOption((prev) => ({ ...prev, [`${word}-${idx}`]: tone, }))} />
                            {tone}
                          </label>
                        ))}
                      </div>
                    </div>
                  ))}
                </div>
              </div>
              <div>
                <h2 style={{ marginTop: "0px", marginLeft: "50px", textAlign: "right" }}>Can you tell from the contours whether <br />your tone pronunciations are correct?</h2>
                {["totally", "most", "some", "few", "not at all"].map((cf) => (
                  <label key={cf} style={{ marginRight: "4px" }}>
                    <input type="radio" name={"confidence"} value={cf} checked={confidence === cf}
                      onChange={() => setConfidence(cf)} />
                    {cf}
                  </label>
                ))}
              </div>
            </div>
            <div style={{ marginTop: "20px" }}>
              <button onClick={handleSubmit} style={{ marginRight: "10px" }}>confirm</button>
              <button onClick={() => setIsModalOpen(false)}>cancel</button>
            </div>
          </div>
        </div>
      )}

      {/*selectedOption && <p>你選的是: {selectedOption}</p>*/}
    </div>
  );
}
function showwave(tag, saveBDflag, wavBlob, setboundaries, reading_times, used_region, used_pitch) {

  const container = document.querySelector('#' + tag)
  const regions = RegionsPlugin.create()
  const wavbloburl = URL.createObjectURL(wavBlob)
  regions.id = 'regionplugin'
  const wavesurfer_recorded = WaveSurfer.create({
    container,
    sampleRate: 16000,
    waveColor: 'rgb(200, 100, 0)',
    progressColor: 'rgba(255, 190, 124, 1)',
    url: wavbloburl,
    plugins: [TimelinePlugin.create({
      height: 15,
      timeInterval: 0.1,
      primaryLabelInterval: 1,
      style: {
        fontSize: '15px',
        color: 'rgb(0, 0, 0)',
      },
    }),
    Minimap.create({
      height: 20,
      waveColor: '#ddd',
      progressColor: '#999',
      // the Minimap takes all the same options as the WaveSurfer itself
    }),
    Hover.create({
      //formatTimeCallback?: ((seconds) => string);
      formatTimeCallback: (seconds) => {
        return seconds.toFixed(3) + 's'
      },
      lineColor: '#ff0000',
      lineWidth: 2,
      labelBackground: '#555',
      labelColor: '#fff',
      labelSize: '11px',
      labelPreferLeft: false,
    }),
      regions]
  })
  wavesurfer_recorded.sylnum = 0

  const handleZoom = (e) => {
    if (wavesurfer_recorded && wavesurfer_recorded.getDuration() > 0) {
      const minPxPerSec = e.target.valueAsNumber
      wavesurfer_recorded.zoom(minPxPerSec)
    }
  }
  const handlePlayPause = () => {
    const targetdiv = document.querySelector('#recordingsexp').querySelector('div').shadowRoot.querySelector('[part="minimap"]').querySelector('div').nextElementSibling.shadowRoot.querySelector('div').querySelector('div').querySelector('div')
    const polygon = targetdiv.style.clipPath.match(/[\d.]+%/g)
    let target_region = ""
    let inregion = false
    let current_time_idx = 0.0
    if (wavesurfer_recorded.plugins[3].regions.length === wavesurfer_recorded.sylnum + 1) {
      target_region = wavesurfer_recorded.plugins[3].regions[wavesurfer_recorded.sylnum]
      if (polygon) {
        current_time_idx = Number(polygon[0].slice(0, -1)) * 0.01 * target_region.totalDuration
        inregion = (current_time_idx > target_region.start) && (current_time_idx < target_region.end)
      }
    }

    if (wavesurfer_recorded.regionplaying === null) {
      wavesurfer_recorded.regionplaying = false
    }
    if (wavesurfer_recorded.plugins[3].regions.length === wavesurfer_recorded.sylnum + 1) {
      //wavesurfer_recorded.plugins[3].regions[wavesurfer_recorded.sylnum].play(true)
      if (inregion) {
        if (wavesurfer_recorded.regionplaying === false) {
          wavesurfer_recorded?.play(current_time_idx, target_region.end)
          wavesurfer_recorded.regionplaying = true
        }
        else {
          wavesurfer_recorded?.pause()
          wavesurfer_recorded.regionplaying = false
        }
      }
      else {
        wavesurfer_recorded.plugins[3].regions[wavesurfer_recorded.sylnum].play(true)
        wavesurfer_recorded.regionplaying = true
      }
    }
    else {
      wavesurfer_recorded?.playPause()
    }
  }
  const handleregion = (e) => {
    //console.log(e)
    let bdinfo = ""
    for (let i = 0; i <= e.regions.length - 1; i++) {
      //console.log(e.regions[i].start,e.regions[i].content.innerText)
      if (e.regions[i].content !== undefined) {
        bdinfo = bdinfo + e.regions[i].start + ' ' + e.regions[i].content.innerText + '\n'
      }
    }
    //onsole.log('bdinfo',bdinfo)
    
    if (saveBDflag) {
      setboundaries(bdinfo)
    }
    
  }
  wavesurfer_recorded.renderer.container.addEventListener('contextmenu', e => {
    e.preventDefault()
    //showrbMenu(e.clientX,e.clientY,region,ws)
    //console.log(ws.plugins[2],ws.plugins[2].lastPointerPosition,ws.plugins[2].wrapper,ws.plugins[2].wrapper.querySelector('span').textContent)
    //console.log(ws.plugins[2].lastPointerPosition.clientX,ws.plugins[2].lastPointerPosition.clientY,ws.plugins[2].wrapper.querySelector('span').textContent)
    const clientX = wavesurfer_recorded.plugins[2].lastPointerPosition.clientX
    const clientY = wavesurfer_recorded.plugins[2].lastPointerPosition.clientY
    const current_time = wavesurfer_recorded.plugins[2].wrapper.querySelector('span').textContent.slice(0, -1)
    //console.log(clientX,clientY,current_time)

    showabMenu(clientX, clientY, current_time, wavesurfer_recorded)
  })
  wavesurfer_recorded.once("ready", () => {
    URL.revokeObjectURL(wavbloburl)
    handleZoom({ target: { valueAsNumber: 200 } }) // 初始 zoom
    if (used_region) {
      //console.log('used_region', used_region)
      dsb.drawsylboundaries({ ws: wavesurfer_recorded, url: "", sd: used_region })
    }
    if (used_pitch) {
      //console.log('used_pitch', used_pitch.recordings[0])
      //dp.pitchdraw({ ws: wavesurfer_recorded, url: "", pd: used_pitch.recordings[0] })
      setTimeout(() => {
        dp.pitchdraw({ ws: wavesurfer_recorded, url: "", pd: used_pitch })
      }, 200)
    }
  })

  wavesurfer_recorded.on('interaction', () => {
    dsb.clearlastregion({ ws: wavesurfer_recorded })
    //regions.clearRegions()
  })
  wavesurfer_recorded.on("zoom", () => {
    dp.zoompitchdraw({ ws: wavesurfer_recorded })
  })
  regions.enableDragSelection({
    color: 'rgba(255, 0, 0, 0.1)',
    drag: false,
  })
  regions.on("region-initialized", () => {
    dsb.clearlastregion({ ws: wavesurfer_recorded })
    //regions.clearRegions()
  })
  regions.on('region-created', (region) => {
    // region.element 是區段的 <div>
    handleregion(regions)
    const label = region.element.querySelector(('[part="region-content"]'))
    if (!label) {
      return
    }

    //console.log(label, label.textContent, label.textContent.length)
    if (label.textContent === 'SIL') {
      label.textContent = 'ϵ'
    }
    label.style.position = 'relative'
    label.style.right = label.textContent.length * 0.7 + 'em'
    label.style.fontFamily = '"Times New Roman", Times, serif'
    region.element.addEventListener('contextmenu', e => {
      e.preventDefault()
      e.stopPropagation()
      showrbMenu(e.clientX, e.clientY, region, wavesurfer_recorded)
      //console.log(ws)
    })
  })
  regions.on('region-removed', (region) => {
    handleregion(regions)
  })
  regions.on('region-updated', (region) => {
    handleregion(regions)
  })
  const button = container.appendChild(document.createElement('button'))
  button.textContent = 'Play'
  button.addEventListener("click", handlePlayPause)
  //button.onclick = () => wavesurfer_recorded.playPause()
  wavesurfer_recorded.on('pause', () => (button.textContent = 'Play'))
  wavesurfer_recorded.on('play', () => (button.textContent = 'Pause'))
  // Download link
  const link = container.appendChild(document.createElement('a'))
  Object.assign(link, {
    href: URL.createObjectURL(wavBlob),
    download: 'recording.' + wavBlob.type.split(';')[0].split('/')[1] || 'wav',
    textContent: 'Download recording',
  })
  const zoominout = container.appendChild(document.createElement('input'))
  zoominout.addEventListener("input", handleZoom)
  Object.assign(zoominout, {
    type: 'range',
    min: "20",
    max: "1000",
    defaultValue: "200",
    //onChange:{handleZoom}
  })
  const canvas = document.createElement('canvas')
  const ctx = canvas.getContext('2d')
  ctx.font = '14px Times New Roman';
  ctx.fillStyle = 'blue';
  ctx.textAlign = 'left';
  ctx.textBaseline = 'top';
  ctx.fillText('record times: ' + (reading_times + 1), 0, 0);
  wavesurfer_recorded.renderer.getWrapper().appendChild(canvas)

  return wavesurfer_recorded
}
function Recording({ tag, Fileidx, exp_group, profile, setshowed_posterior, setboundaries, reading_times, setReading_Times, saved_regions, setSaved_Regions, saved_pitchs, setSaved_Pitchs, setRecord_Enable, setNext_Enable }) {
  let wavesurfer_recorded = null
  const wavesurferRef = useRef(null)
  const lastFileidxRef = useRef(null)
  function addRegion(newItem) {
    setSaved_Regions(prev => {
      const next = [...prev];
      next[Fileidx] = [...next[Fileidx], newItem];
      return next;
    });
  }
  function addPitch(newItem, idx) {
    //console.log('addPitch', idx)
    setSaved_Pitchs(prev => {
      const next = [...prev];
      if (idx === next[Fileidx].length) {
        next[Fileidx] = [...next[Fileidx], newItem];
      }
      else {
        next[Fileidx][idx] = newItem;
      }
      return next;
    });
  }
  useEffect(() => {

    loadAllRecordings((allRecordings) => {
      if (Fileidx === "") return
      if (lastFileidxRef.current === Fileidx) return
      //console.log("saved_regions",saved_regions)
      lastFileidxRef.current = Fileidx
      const wavlst = []
      allRecordings.forEach(r => {
        //const url = URL.createObjectURL(r.blob)
        // 用 WaveSurfer 或 HTML audio 播放
        if (r.Fileidx === Fileidx) {
          wavlst.push(r.wavBlob)
        }
        //console.log("Loaded recording", r, URL.createObjectURL(r.wavBlob))
        //showwave(tag, Fileidx, r.wavBlob)
      })

      //console.log(saved_pitchs[Fileidx])
      for (let i = wavlst.length - 1; i >= 0; i--) {
        //console.log(saved_regions[Fileidx][i], useful_pitchs[i], i);
        let saveBDflag = true
        if (i !== wavlst.length - 1) {
          saveBDflag = false
        }
        wavesurfer_recorded = showwave(tag, saveBDflag, wavlst[i], setboundaries, i, saved_regions[Fileidx][i], saved_pitchs[Fileidx][i])
        if (i == wavlst.length - 1) {
          wavesurferRef.current = wavesurfer_recorded
        }
      }
    })

    //console.log('here', tag, Fileidx)
    let scrollingWaveform = false
    let continuousWaveform = true
    let wavesurfer, record
    let recordtimes = 0
    let wavBlob, wavurl

    const mic = document.querySelector('#mic')
    if (mic && mic.childNodes.length > 0) {
      mic.style.height = '0px'
      mic.innerHTML = ''
    }

    const createWaveSurfer = () => {
      // Destroy the previous wavesurfer instance
      if (wavesurfer) {
        wavesurfer.destroy()
      }

      // Create a new Wavesurfer instance
      wavesurfer = WaveSurfer.create({
        container: '#mic',
        sampleRate: 16000,
        waveColor: 'rgb(200, 0, 200)',
        progressColor: 'rgb(100, 0, 100)',
      })

      // Initialize the Record plugin
      record = wavesurfer.registerPlugin(
        RecordPlugin.create({
          audioBitsPerSecond: 256000,
          //sampleRate: 16000,
          renderRecordedAudio: false,
          scrollingWaveform,
          continuousWaveform,
          continuousWaveformDuration: 30, // optional
        }),
      )

      record.on('record-start', () => {
        //console.log(record.stream)
        recordtimes = recordtimes + 1

        document.querySelector('#mic').style.display = 'inline'
      })
      // Render recorded audio
      record.on('record-end', (blob) => {
        document.querySelector('#mic').style.display = 'none'
        const container = document.querySelector('#' + tag)
        //const recordedUrl = URL.createObjectURL(blob)
        // Create wavesurfer from the recorded audio
        //const regions = RegionsPlugin.create()
        //regions.id='regionplugin'
        //console.log(wavurl)
        wavesurfer_recorded = showwave(tag, true, wavBlob, setboundaries, reading_times[Fileidx], null, null)
        wavesurferRef.current = wavesurfer_recorded


        //console.log(container,record,container.querySelectorAll('div'),container.querySelectorAll('a'),container.querySelectorAll('button'))
        let wavdivs = container.querySelectorAll('div')
        const linkas = container.querySelectorAll('a')
        const playbuttons = container.querySelectorAll('button')
        const zoominouts = container.querySelectorAll('input')
        container.insertBefore(wavdivs[wavdivs.length - 1], wavdivs[0])
        wavdivs = container.querySelectorAll('div')
        if (wavdivs.length > 1) {
          container.insertBefore(playbuttons[playbuttons.length - 1], wavdivs[1])
          container.insertBefore(linkas[linkas.length - 1], wavdivs[1])
          container.insertBefore(zoominouts[zoominouts.length - 1], wavdivs[1])
        }

      })
      pauseButton.style.display = 'none'
      recButton.textContent = 'Record'

      record.on('record-progress', (time) => {
        updateProgress(time)
      })

      websocket.onmessage = (event) => {
        //console.log("server 回傳：", event.data)
        if (event.data.slice(0, 18) == "syllable_boundary:") {
          openDB((db) => {
            const tx = db.transaction("recordings", "readwrite")
            const store = tx.objectStore("recordings")
            const RTs = reading_times[Fileidx]
            store.put({ wavBlob, time: Date.now(), Fileidx, reading_times: RTs })  // 物件存入
            tx.oncomplete = () => console.log("Saved recording")
          })
          //console.log(reading_times)
          //reading_times[Fileidx] = reading_times[Fileidx] +1
          //setReading_Times(reading_times)
          setReading_Times(prev => {
            const updated = [...prev]
            updated[Fileidx] = updated[Fileidx] + 1
            return updated
          })
          dsb.drawsylboundaries({ ws: wavesurferRef.current, url: "", sd: event.data.slice(18, -1) })
          //setboundaries(event.data.slice(18, -1))

          addRegion(event.data.slice(18, -1))
          //console.log(saved_regions)

          if (Fileidx >= 20) {
            setRecord_Enable(false)
            setNext_Enable(false)
          }
        }
        else if (event.data.slice(0, 6) == "pitch:") {
          //console.log("HERE:", event.data.slice(6, -1))
          dp.pitchdraw({ ws: wavesurferRef.current, url: "", pd: event.data.slice(6, -1) })
          addPitch(event.data.slice(6, -1), reading_times[Fileidx] - 1)
        }
        else if (event.data.slice(0, 9) == "postrior:") {
          //console.log(event.data)
          const get_GOP = (p, t) => {
            const tv = p[t - 1]
            //p.splice(p[t - 1], 1)
            const newp = p.filter((_, index) => index !== t - 1)
            
            newp.sort((a, b) => b - a)
            console.log(newp)
            return Math.log(tv / newp[0])
          }
          const tempStrlst = exp_pinyin_list[Fileidx].replaceAll(" /", "").split(" ")
          const postriors = event.data.slice(9, -1).split("\n")
          let showedtext = ""
          //console.log(tempStrlst)
          for (let i = 0; i <= tempStrlst.length - 1; i++) {
            const GOPmean = GOPdist[tempStrlst[i]][1]
            const GOPstd = GOPdist[tempStrlst[i]][2]
            const postrior = postriors[i].slice(postriors[i].indexOf(":") + 2, -1).split(" ").map(item => parseFloat(item))
            const tone = tempStrlst[i][tempStrlst[i].length - 1]
            
            const gop = get_GOP(postrior, tone)
            let goodness = ''
            if (gop > GOPmean + GOPstd) {
              goodness='Good'
            }
            else if (gop > GOPmean - GOPstd) {
              goodness = 'Ok'
            }
            else {
              goodness = 'Bad'
            }
            console.log(tempStrlst[i], postrior, tone, gop, goodness, GOPmean, GOPstd)
            showedtext = showedtext + goodness + "|" + tempStrlst[i].slice(0, -1) + ": " + postrior.map(n => (n*100).toFixed(1)).join(" ") + '\n'
          }
          setshowed_posterior(prev => {
            const updated = [...prev]
            //updated[Fileidx] = event.data.slice(9, -1)
            updated[Fileidx] = showedtext
            return updated
          })
        }

      }
    }


    const progress = document.querySelector('#progress')
    const updateProgress = (time) => {
      // time will be in milliseconds, convert it to mm:ss format
      const formattedTime = [
        Math.floor((time % 3600000) / 60000), // minutes
        Math.floor((time % 60000) / 1000), // seconds
      ]
        .map((v) => (v < 10 ? '0' + v : v))
        .join(':')
      progress.textContent = formattedTime
    }

    const pauseButton = document.querySelector('#pause')
    pauseButton.onclick = () => {
      if (record.isPaused()) {
        record.resumeRecording()
        pauseButton.textContent = 'Pause'
        return
      }

      record.pauseRecording()
      pauseButton.textContent = 'Resume'
    }

    const micSelect = document.querySelector('#mic-select')
    // Mic selection
    RecordPlugin.getAvailableAudioDevices().then((devices) => {
      devices.forEach((device) => {
        const option = document.createElement('option')
        option.value = device.deviceId
        option.text = device.label || device.deviceId
        micSelect.appendChild(option)
      })
    })

    // Record button
    const recButton = document.querySelector('#record')
    recButton.onclick = () => {
      if (record.isRecording() || record.isPaused()) {
        //console.log(recorderData)
        wavBlob = wlp.stopRecording(wlp.recorderData)
        //openDB((db) => {
        //  const tx = db.transaction("recordings", "readwrite")
        //  const store = tx.objectStore("recordings")
        //  store.put({ wavBlob, time: Date.now(), Fileidx, reading_times })  // 物件存入
        //  tx.oncomplete = () => console.log("Saved recording")
        //})
        console.log('wavid' + profile + ' ' + (reading_times[Fileidx] + 1), exp_file_list[Fileidx], 'context' + exp_word_list[Fileidx], 'group' + exp_group)
        get_BPT(wavBlob, 'wavid' + profile + '_' + exp_file_list[Fileidx] + ' ' + (reading_times[Fileidx] + 1), 'context' + exp_word_list[Fileidx], 'group' + exp_group)
        wavurl = URL.createObjectURL(wavBlob);
        record.stopRecording()
        recButton.textContent = 'Record'
        pauseButton.style.display = 'none'
        pauseButton.textContent = 'Pause'
        return
      }

      recButton.disabled = true

      // reset the wavesurfer instance

      // get selected device
      const deviceId = micSelect.value
      //console.log(record)
      //record.startRecording({
      //  deviceId,
      //  constraints: {
      //    audio: {
      //      deviceId: deviceId ? { exact: deviceId } : undefined,
      //      echoCancellation: false,
      //      noiseSuppression: false,
      //      autoGainControl: false,
      //      channelCount: 1,
      //      sampleRate: 16000,
      //      advanced: [
      //        { echoCancellation: false },
      //        { noiseSuppression: false },
      //        { autoGainControl: false }
      //      ],
      //    }
      //  }
      //}).then(() => {
      //  recButton.textContent = 'Stop'
      //  recButton.disabled = false
      //  pauseButton.style.display = 'inline'
      //})
      record.startRecording({ deviceId }).then(() => {
        wlp.StartWorklet(record.stream)
        recButton.textContent = 'Stop'
        recButton.disabled = false
        pauseButton.style.display = 'inline'
      })
    }

    document.querySelector('#scrollingWaveform').onclick = (e) => {
      scrollingWaveform = e.target.checked
      if (continuousWaveform && scrollingWaveform) {
        continuousWaveform = false
        document.querySelector('#continuousWaveform').checked = false
      }
      createWaveSurfer()
    }

    document.querySelector('#continuousWaveform').onclick = (e) => {
      continuousWaveform = e.target.checked
      if (continuousWaveform && scrollingWaveform) {
        scrollingWaveform = false
        document.querySelector('#scrollingWaveform').checked = false
      }
      createWaveSurfer()
    }
    createWaveSurfer()
  }, [reading_times])

}
document.addEventListener('click', e => {
  const menu = document.getElementById('syllable-menu')
  if (menu && !menu.contains(e.target)) {
    removeContextMenu()
  }
})
function addBoundaryAtTime(x, y, time, ws) {
  let rp = 0
  for (let i = 0; i <= ws.plugins.length - 1; i++) {
    if (ws.plugins[i].id === 'regionplugin') {
      rp = ws.plugins[i]
    }
  }

  const menu = document.getElementById('syllable-menu')
  if (!menu) return

  // 清掉原本選項
  menu.innerHTML = ''

  const input = document.createElement('input')
  input.type = 'text'
  input.placeholder = 'syllable'
  input.style.width = '120px'
  input.style.fontSize = '14px'
  input.style.padding = '4px'

  menu.appendChild(input)

  input.focus()

  const submit = () => {
    const value = input.value.trim()
    if (!value) {
      removeContextMenu()
      return
    }

    rp.addRegion({
      start: time,
      content: value,
      color: 'rgba(0, 0, 0, 0.3)',
      drag: true,
    })
    ws.sylnum = ws.sylnum + 1
    removeContextMenu()
  }

  // Enter 確認
  input.addEventListener('keydown', e => {
    if (e.key === 'Enter') submit()
    if (e.key === 'Escape') removeContextMenu()
  })

  // 點別處或失焦取消 / 確認（你可自行選）
  input.addEventListener('blur', () => { })
}
function showabMenu(x, y, time, ws) {
  removeContextMenu()

  const menu = document.createElement('div')
  menu.id = 'syllable-menu'
  menu.style.position = 'fixed'
  menu.style.left = `${x}px`
  menu.style.top = `${y}px`
  menu.style.background = '#fff'
  menu.style.border = '1px solid #ccc'
  menu.style.padding = '4px'
  menu.style.zIndex = 10000

  menu.addEventListener('click', e => e.stopPropagation())
  menu.addEventListener('contextmenu', e => e.stopPropagation())

  const add = document.createElement('div')
  add.innerText = 'add boundary'
  add.style.cursor = 'pointer'

  add.onclick = () => {
    addBoundaryAtTime(x, y, time, ws)
  }

  menu.appendChild(add)
  document.body.appendChild(menu)
}
function removeContextMenu() {
  const old = document.getElementById('syllable-menu')
  if (old) old.remove()
}
function showrbMenu(x, y, boundary, ws) {
  removeContextMenu()
  //console.log(x,y,boundary)
  const menu = document.createElement('div')
  menu.id = 'syllable-menu'
  menu.style.position = 'fixed'
  menu.style.left = `${x}px`
  menu.style.top = `${y}px`
  menu.style.background = '#fff'
  menu.style.border = '1px solid #ccc'
  menu.style.padding = '4px'
  menu.style.zIndex = 10000

  const del = document.createElement('div')
  del.innerText = 'del boundary'
  del.style.cursor = 'pointer'

  del.onclick = () => {
    //console.log('click')
    removeContextMenu()
    boundary.remove()
    ws.sylnum = ws.sylnum - 1
  }

  menu.appendChild(del)
  document.body.appendChild(menu)
}
function WavePlayer({ url }) {
  const containerRef = useRef(null)
  const wavesurferRef = useRef(null)


  useEffect(() => {
    const regions = RegionsPlugin.create()
    if (!containerRef.current) return
    regions.id = 'regionplugin'
    const ws = WaveSurfer.create({
      container: containerRef.current,
      waveColor: "rgba(255, 132, 255, 1)",
      progressColor: "rgba(200, 0, 200, 0.2)",
      responsive: true,
      url,
      plugins: [TimelinePlugin.create({
        height: 15,
        timeInterval: 0.1,
        primaryLabelInterval: 1,
        style: {
          fontSize: '15px',
          color: 'rgb(0, 0, 0)',
        },
      }),
      Minimap.create({
        height: 20,
        waveColor: '#ddd',
        progressColor: '#999',
        // the Minimap takes all the same options as the WaveSurfer itself
      }),
      Hover.create({
        //formatTimeCallback?: ((seconds) => string);
        formatTimeCallback: (seconds) => { return seconds.toFixed(3) + 's' },
        lineColor: '#ff0000',
        lineWidth: 2,
        labelBackground: '#555',
        labelColor: '#fff',
        labelSize: '11px',
        labelPreferLeft: false,
      }),
        regions,],
    })

    wavesurferRef.current = ws
    ws.renderer.container.addEventListener('contextmenu', e => {
      e.preventDefault()
      //showrbMenu(e.clientX,e.clientY,region,ws)
      //console.log(ws.plugins[2],ws.plugins[2].lastPointerPosition,ws.plugins[2].wrapper,ws.plugins[2].wrapper.querySelector('span').textContent)
      //console.log(ws.plugins[2].lastPointerPosition.clientX,ws.plugins[2].lastPointerPosition.clientY,ws.plugins[2].wrapper.querySelector('span').textContent)
      const clientX = ws.plugins[2].lastPointerPosition.clientX
      const clientY = ws.plugins[2].lastPointerPosition.clientY
      const current_time = ws.plugins[2].wrapper.querySelector('span').textContent.slice(0, -1)
      //console.log(clientX,clientY,current_time)

      showabMenu(clientX, clientY, current_time, ws)
    })
    ws.once("ready", () => {
      const pitchurl = url.substring(0, url.indexOf('wav')) + 'txt'
      dp.pitchdraw({ ws, url: pitchurl, pd: "" })
      handleZoom({ target: { valueAsNumber: 200 } }) // 初始 zoom
      const sylboundariesurl = url.substring(0, url.indexOf('wav')) + 'lab'
      dsb.drawsylboundaries({ ws, url: sylboundariesurl, sd: "" })
    })
    ws.on('interaction', (newTime) => {
      dsb.clearlastregion({ ws })
      //regions.clearRegions()
      //console.log(newTime)
      //console.log(ws.renderer.container)
    })
    ws.on("zoom", () => {
      dp.zoompitchdraw({ ws })
    })
    regions.enableDragSelection({
      color: 'rgba(255, 0, 0, 0.1)',
      drag: false,
    })
    regions.on("region-initialized", () => {
      dsb.clearlastregion({ ws })
      //regions.clearRegions()
    })
    regions.on('region-created', (region) => {
      // region.element 是區段的 <div>
      const label = region.element.querySelector(('[part="region-content"]'))
      if (!label) {
        return
      }
      //console.log(label, label.textContent, label.textContent.length)
      if (label.textContent === 'SIL') {
        label.textContent = 'ϵ'
      }
      label.style.position = 'relative'
      label.style.right = label.textContent.length * 0.7 + 'em'
      label.style.fontFamily = '"Times New Roman", Times, serif'
      //console.log(region.element)
      region.element.addEventListener('contextmenu', e => {
        e.preventDefault()
        e.stopPropagation()
        showrbMenu(e.clientX, e.clientY, region, ws)
        //console.log(ws)
      })
    })
    return () => ws.destroy()
  }, [url])

  const handlePlayPause = () => {
    if (wavesurferRef.current.plugins[3].regions.length === wavesurferRef.current.sylnum + 1) {
      wavesurferRef.current.plugins[3].regions[wavesurferRef.current.sylnum].play(true)
    }
    else {
      wavesurferRef.current?.playPause()
    }
  }

  const handleZoom = (e) => {
    if (wavesurferRef.current && wavesurferRef.current.getDuration() > 0) {
      const minPxPerSec = e.target.valueAsNumber
      wavesurferRef.current.zoom(minPxPerSec)
    }
  }

  return (
    <div
      style={{
        width: "80vw",
        background: "white",
        borderRadius: "10px",
        padding: "1rem",
        marginBottom: "2rem",
      }
      }
    >
      <div ref={containerRef} style={{ width: "100%" }}> </div>
      < input
        type="range"
        min="20"
        max="1000"
        defaultValue="200"
        onInput={handleZoom}
      />
      <button onClick={handlePlayPause}> Play / Pause </button>
    </div>
  )
}
function WavePlayer_example({ url, EG }) {
  const containerRef = useRef(null)
  const wavesurferRef = useRef(null)


  useEffect(() => {
    const regions = RegionsPlugin.create()
    if (!containerRef.current) return
    regions.id = 'regionplugin'
    const ws = WaveSurfer.create({
      container: containerRef.current,
      waveColor: "rgba(255, 132, 255, 1)",
      progressColor: "rgba(200, 0, 200, 0.2)",
      responsive: true,
      url,
      plugins: [TimelinePlugin.create({
        height: 15,
        timeInterval: 0.1,
        primaryLabelInterval: 1,
        style: {
          fontSize: '15px',
          color: 'rgb(0, 0, 0)',
        },
      }),
      Minimap.create({
        height: 20,
        waveColor: '#ddd',
        progressColor: '#999',
        // the Minimap takes all the same options as the WaveSurfer itself
      }),
      Hover.create({
        //formatTimeCallback?: ((seconds) => string);
        formatTimeCallback: (seconds) => {
          return seconds.toFixed(3) + 's'
        },
        lineColor: '#ff0000',
        lineWidth: 2,
        labelBackground: '#555',
        labelColor: '#fff',
        labelSize: '11px',
        labelPreferLeft: false,
      }),
        regions,],
    })

    wavesurferRef.current = ws
    ws.once("ready", () => {
      setTimeout(() => {
        let pitchurl = ""
        if (EG === 'PC') {
          pitchurl = url.substring(0, url.indexOf('wav')) + 'PC'
        }
        else {
          pitchurl = url.substring(0, url.indexOf('wav')) + 'txt'
        }
        dp.pitchdraw({ ws, url: pitchurl, pd: "" })
        const sylboundariesurl = url.substring(0, url.indexOf('wav')) + 'lab'
        dsb.drawsylboundaries({ ws, url: sylboundariesurl, sd: "" })
      }, 200)
      handleZoom({ target: { valueAsNumber: 200 } }) // 初始 zoom
    })
    ws.on('interaction', () => {
      dsb.clearlastregion({ ws })
      //regions.clearRegions()
    })
    ws.on("zoom", () => {
      dp.zoompitchdraw({ ws })
    })
    regions.enableDragSelection({
      color: 'rgba(255, 0, 0, 0.1)',
      drag: false,
    })
    regions.on("region-initialized", () => {
      dsb.clearlastregion({ ws })
      //regions.clearRegions()
    })
    regions.on('region-created', (region) => {
      // region.element 是區段的 <div>
      const label = region.element.querySelector(('[part="region-content"]'))
      if (!label) {
        return
      }
      //console.log(label, label.textContent, label.textContent.length)
      if (label.textContent === 'SIL') {
        label.textContent = 'ϵ'
      }
      label.style.position = 'relative'
      label.style.right = label.textContent.length * 0.7 + 'em'
      label.style.fontFamily = '"Times New Roman", Times, serif'
    })
    return () => ws.destroy()
  }, [url])

  const handlePlayPause = () => {
    if (wavesurferRef.current.plugins[3].regions.length === wavesurferRef.current.sylnum + 1) {
      wavesurferRef.current.plugins[3].regions[wavesurferRef.current.sylnum].play(true)
    }
    else {
      wavesurferRef.current?.playPause()
    }
  }

  const handleZoom = (e) => {
    if (wavesurferRef.current && wavesurferRef.current.getDuration() > 0) {
      const minPxPerSec = e.target.valueAsNumber
      wavesurferRef.current.zoom(minPxPerSec)
    }
  }

  return (
    <div
      style={{
        width: "80vw",
        background: "white",
        borderRadius: "10px",
        padding: "1rem",
        marginBottom: "2rem",
      }
      }
    >
      <div ref={containerRef} style={{ width: "100%" }}> </div>
      < input
        type="range"
        min="20"
        max="1000"
        defaultValue="200"
        onInput={handleZoom}
      />
      <button onClick={handlePlayPause}> Play / Pause </button>
    </div>
  )
}
function Welcomeinterface({ setPI, setA, setG, setM, setS, setT, setEG }) {
  return (
    <div>
      <input
        type="text"
        placeholder="Participant Id"
        onChange={(e) => setPI(e.target.value)}
        style={{ padding: "0.5em", marginRight: "1em" }}
      />
      <br></br>
      <input
        type="text"
        placeholder="Age"
        onChange={(e) => setA(e.target.value)}
        style={{ padding: "0.5em", marginRight: "1em" }}
      />
      <br></br>
      Gender:
      <input type="radio" name="Gender" value="Male" onChange={(e) => setG(e.target.value)} />Male
      <input type="radio" name="Gender" value="Female" onChange={(e) => setG(e.target.value)} />Female
      <br></br>
      <input
        type="text"
        placeholder="Mother Language"
        onChange={(e) => setM(e.target.value)}
        style={{ padding: "0.5em", marginRight: "1em" }}
      />
      <br></br>
      <input
        type="text"
        placeholder="Study Chinese for ? years"
        onChange={(e) => setS(e.target.value)}
        style={{ padding: "0.5em", marginRight: "1em" }}
      />
      <br></br>
      TOCFL:
      <input type="radio" name="TOCFL" value="A" onChange={(e) => setT(e.target.value)} />A
      <input type="radio" name="TOCFL" value="B" onChange={(e) => setT(e.target.value)} />B
      <br></br>
      Group:
      <input type="radio" name="Group" value="PC" onChange={(e) => setEG(e.target.value)} />PC
      <input type="radio" name="Group" value="PPC" onChange={(e) => setEG(e.target.value)} />PPC

    </div>
  )
}
const exp_file_list = ['mono_example_fan1', 'mono_example_wu2', 'mono_example_kou3', 'mono_example_chu4',
  'di_example_yi1yi1', 'di_example_yi1fu2', 'di_example_ji1tui3', 'di_example_you1xiu4',
  'di_example_shi2zhuo1', 'di_example_wu2du2', 'di_example_fang2ka3', 'di_example_fu2di4',
  'di_example_wo3ma1', 'di_example_ai3fang2', 'di_example_qian3san4', 'di_example_san4kai1',
  'di_example_she4ji2', 'di_example_ci4deng3', 'di_example_wu4chu4', 'di_example_yi2ge5',
  'phrase_example_1', 'phrase_example_2', 'phrase_example_3', 'phrase_example_4', 'phrase_example_5', 'phrase_example_6', 'phrase_example_7',
  'phrase_example_8', 'phrase_example_9', 'phrase_example_10', 'phrase_example_11', 'phrase_example_12', 'phrase_example_13', 'phrase_example_14',
  'sentence_example_1', 'sentence_example_2', 'sentence_example_3', 'sentence_example_4', 'sentence_example_5', 'sentence_example_6', 'sentence_example_7',
]
const exp_word_list = ['翻', '無', '口', '觸',
  '依依', '衣服', '雞腿', '優秀', '石桌', '無獨', '房卡', '福地', '我媽', '矮房', '遣散', '散開', '射擊', '次等', '誤觸', '一個',
  '雨傘桶', '雞腿湯', '門口旁', '有一個', '誤觸了', '遣散了', '快散開', '不小心', '依依不捨', '又出現了', '次等職員', '無獨有偶', '放在石桌上', '打翻在衣服上',
  '次等職員都被遣散了。', '我媽把房卡放在石桌上。', '爺爺對這片福地依依不捨。', '快散開，他誤觸了射擊開關。',
  '前面的矮房門口旁有一個雨傘桶。', '無獨有偶，又出現了一個優秀的人才。', '她買的雞腿湯不小心打翻在衣服上了。'
]
const exp_pinyin_list = ['fan1', 'wu2', 'kou3', 'chu4',
  'yi1 yi1', 'yi1 fu2', 'ji1 tui3', 'you1 xiu4', 'shi2 zhuo1', 'wu2 du2', 'fang2 ka3', 'fu2 di4',
  'wo3 ma1', 'ai3 fang2', 'qian3 san4', 'san4 kai1', 'she4 ji2', 'ci4 deng3', 'wu4 chu4', 'yi2 ge5',
  'yu2 san2 tong3', 'ji1 tui3 tang1', 'men2 kou3 pang2', 'you3 yi2 ge5', 'wu4 chu4 le5', 'qian3 san4 le5', 'kuai4 san4 kai1', 'bu4 xiao3 xin1', 'yi1 yi1 bu4 she3', 'you4 chu1 xian4 le5', 'ci4 deng3 zhi2 yuan2', 'wu2 du2 you2 ou3', 'fang4 zai4 / shi2 zhuo1 shang4', 'da3 fan1 zai4 / yi1 fu2 shang4',
  'ci4 deng3 / zhi2 yuan2 / dou1 bei4 / qian3 san4 le5', 'wo3 ma1 / ba3 fang2 ka3 / fang4 zai4 / shi2 zhuo1 shang4',
  'ye2 ye5 / dui4 zhe4 pian4 fu2 di4 / yi1 yi1 bu4 she3', 'kuai4 san4 kai1 / ta1 wu4 chu4 le5 / she4 ji2 kai1 guan1',
  'qian2 mian4 de5 / ai3 fang2 men2 kou3 pang2 / you3 yi2 ge5 yu2 san2 tong3',
  'wu2 du2 you2 ou3 / you4 chu1 xian4 le5 yi2 ge5 / you1 xiu4 de5 ren2 cai2',
  'ta1 mai3 de5 / ji1 tui3 tang1 / bu4 xiao3 xin1 da3 fan1 zai4 / yi1 fu2 shang4 le5',
]
function Exp_start({ PI, A, G, M, S, T, EG, setDisplayImage, setwavePlayer_url, setshowed_text, Fileidx, setshowed_pinyin }) {
  console.log(PI, A, G, M, S, T, EG)
  const imageMap = {
    PC: "exp_data/five_tone_value.png",
    PPC: "exp_data/standard_tone.png",
  }
  setDisplayImage(imageMap[EG])

  //document.body.appendChild(WavePlayer("./exp_data/mono_example_fan1.wav"))
  //document.querySelector('.App').appendChild(WavePlayer("./exp_data/mono_example_fan1.wav"))
  setshowed_text(exp_word_list[Fileidx])
  setshowed_pinyin(exp_pinyin_list[Fileidx])
  setwavePlayer_url("exp_data/" + exp_file_list[Fileidx] + ".wav")
}
function handlenextwav(setFileidx, Fileidx, setwavePlayer_url, setshowed_text, setshowed_pinyin) {
  setshowed_text(exp_word_list[Fileidx + 1])
  setshowed_pinyin(exp_pinyin_list[Fileidx + 1])
  setwavePlayer_url("exp_data/" + exp_file_list[Fileidx + 1] + ".wav")
  setFileidx(Fileidx + 1)
}
function handlegotoidx(target_idx, setFileidx, setwavePlayer_url, setshowed_text, setshowed_pinyin) {
  setshowed_text(exp_word_list[target_idx])
  setshowed_pinyin(exp_pinyin_list[target_idx])
  setwavePlayer_url("exp_data/" + exp_file_list[target_idx] + ".wav")
  setFileidx(Number(target_idx))
}
function handlerestart(setParticipant_Id, setAge, setGender, setMother_Language, setStudy_Years, setTocfl, setExpGroup, setCleared, setFileidx, setshowed_posterior, setReading_Times, setSaved_Regions, setSaved_Pitchs, setRecord_Enable, setNext_Enable) {
  const ok = window.confirm("Are you sure you want to restart? Your current progress will be lost.")
  indexedDB.deleteDatabase("tpt-db")
  localStorage.removeItem("tpt-state")
  if (!ok) return
  setCleared(false)
  setFileidx(0)
  setshowed_posterior(Array(exp_file_list.length).fill(""))
  setParticipant_Id("")
  setAge("")
  setGender("")
  setMother_Language("")
  setStudy_Years("")
  setTocfl("")
  setExpGroup("")
  setSaved_Regions(Array(exp_file_list.length).fill([]))
  setReading_Times(Array(exp_file_list.length).fill(0))
  setSaved_Pitchs(Array(exp_file_list.length).fill([]))
  setRecord_Enable(true)
  setNext_Enable(true)
}
function recompute(boundaries, setboundaries, saved_regions, setSaved_Regions, Fileidx, reading_times) {
  let temp_boundaries = ""
  let temp_reading_times = 0
  //console.log("HERE0", saved_regions[Fileidx].length, saved_regions[Fileidx])

  if (saved_regions[Fileidx].length === 0) {
    return
  }

  //console.log('boundaries', boundaries)
  if (boundaries !== "") {

    const bdtoks = boundaries.split('\n')
      .filter(line => line.trim() !== "") // 過濾空行
      .map(line => line.split(' '));
    //console.log("brefore", bdtoks)
    bdtoks.sort((a, b) => {
      return parseFloat(a[0]) - parseFloat(b[0]);
    })
    //console.log("after", bdtoks)


    for (let i = 0; i < bdtoks.length; i++) {
      if (i === 0) {
        temp_boundaries = "0.0 " + bdtoks[i][0] + " " + bdtoks[i][1] + '\n'
      }
      else {
        temp_boundaries = temp_boundaries + bdtoks[i - 1][0] + " " + bdtoks[i][0] + " " + bdtoks[i][1] + '\n'
      }
    }
    //console.log('temp_boundaries', temp_boundaries)

    setSaved_Regions(prev => {
      const next = [...prev]
      next[Fileidx][next[Fileidx].length - 1] = temp_boundaries
      return next
    })
    temp_reading_times = reading_times
    //console.log("HERE1", saved_regions[Fileidx].length, saved_regions[Fileidx])
    setboundaries("")
  }
  else {
    //console.log("HERE2", saved_regions[Fileidx].length, saved_regions[Fileidx][saved_regions[Fileidx].length - 1])
    temp_boundaries = saved_regions[Fileidx][saved_regions[Fileidx].length - 1]

    temp_reading_times = reading_times
  }

  get_recompute_info('bdinfo|' + exp_file_list[Fileidx] + '|' + String(temp_reading_times) + '|' + temp_boundaries)
}
function ConfirmSubjectInfo({ PI, A, G, M, S, T, EG, setCleared, setDisplayImage, setwavePlayer_url, setshowed_text, Fileidx, setshowed_pinyin, setReading_Times, setUUID }) {
  const allFilled = [PI, A, G, M, S, T, EG].every(v => v && v.trim() !== '')
  //console.log(allFilled)

  const handleConfirm = () => {
    // 清空整個 body 的內容
    //document.body.innerHTML = ''
    setUUID("profile" + PI + "_" + A + "_" + G + "_" + M + "_" + S + "_" + T + "_" + EG)
    //websocket.onopen = () => {
    //  send_profile("profile" + PI + "_" + A + "_" + G + "_" + M + "_" + S + "_" + T + "_" + EG)
    //}
    //send_profile("profile" + PI + "_" + A + "_" + G + "_" + M + "_" + S + "_" + T + "_" + EG)
    setCleared(true)
    //setReading_Times(Array(exp_file_list.length).fill(0))
    Exp_start({ PI, A, G, M, S, T, EG, setDisplayImage, setwavePlayer_url, setshowed_text, Fileidx, setshowed_pinyin })
  }

  return (
    <div>
      {PI} {A} {G} {M} {S} {T} {EG}
      <br />
      <button
        disabled={!allFilled}
        onClick={handleConfirm}
        style={{
          backgroundColor: allFilled ? '#007bff' : '#cccccc', // 有值時藍色，否則灰色
          color: 'white',
          padding: '0.5rem 1rem',
          border: 'none',
          borderRadius: '5px',
          cursor: allFilled ? 'pointer' : 'not-allowed',
          transition: '0.2s'
        }}
      >
        Confirm
      </button>
    </div>
  )
}
function Retrecordhtmls({ tag, Fileidx, exp_group, profile, setshowed_posterior, setboundaries, reading_times, setReading_Times, record_enable, saved_regions, setSaved_Regions, saved_pitchs, setSaved_Pitchs, setUUID, uuid, setRecord_Enable, setNext_Enable }) {
  const lastFileidxRef = useRef(null)

  useEffect(() => {
    if (profile && exp_group) {
      setUUID("profile" + profile + "_" + exp_group)
    }
  }, [profile, exp_group])

  useEffect(() => {
    if (lastFileidxRef.current === Fileidx) return
    lastFileidxRef.current = Fileidx
    if (!uuid) return

    const trySend = () => {
      if (websocket.readyState === WebSocket.OPEN) {
        send_profile(uuid)
      }
    }

    if (websocket.readyState === WebSocket.OPEN) {
      trySend()
    } else {
      websocket.addEventListener("open", trySend, { once: true })
    }
  }, [uuid])

  //useEffect(() => {
  //  setUUID("profile" + profile + "_" + exp_group)
  //  //console.log("uuid:" + uuid)
  //  if (uuid && uuid !== '' && websocket.readyState === WebSocket.OPEN) {
  //    //console.log(uuid)
  //    send_profile(uuid)
  //  }
  //}, [uuid, profile, exp_group]);


  //navigator.mediaDevices.getUserMedia({ audio: { noiseSuppression: false ,echoCancellation: false, autoGainControl: false,} })
  const containerid = "recordings" + tag
  //console.log(containerid, tag)
  return (
    <div
      id="recording_block"
      style={{
        width: "80vw",
        background: "white",
        borderRadius: "10px",
        padding: "1rem",
        marginBottom: "2rem",
        border: "1px solid #ddd",
        marginTop: "1rem"
      }}
    >
      < Recording tag={containerid} Fileidx={Fileidx} exp_group={exp_group} profile={profile} setshowed_posterior={setshowed_posterior} setboundaries={setboundaries} reading_times={reading_times} setReading_Times={setReading_Times} saved_regions={saved_regions} setSaved_Regions={setSaved_Regions} saved_pitchs={saved_pitchs} setSaved_Pitchs={setSaved_Pitchs} setRecord_Enable={setRecord_Enable} setNext_Enable={setNext_Enable} />
      <button disabled={!record_enable} id="record">Record</button>
      <button id="pause" >Pause</button>
      <select id="mic-select">
        <option value="" hidden>Select mic</option>
      </select>
      <label><input type="checkbox" id="scrollingWaveform" /> Scrolling waveform</label>
      <label><input type="checkbox" id="continuousWaveform" checked="checked" onChange={(e) => console.log()} /> Continuous waveform</label>
      <p id="progress">00:00</p>
      <div id="mic" ></div>
      <div id={containerid} ></div>
    </div>
  )
}

//<button onClick={() => setEg("PC")}>切換為 PC 組</button>
export default function Tpt() {
  const [participant_id, setParticipant_Id] = useState("")
  const [age, setAge] = useState("")
  const [gender, setGender] = useState("")
  const [mother_language, setMother_Language] = useState("")
  const [study_years, setStudy_Years] = useState("")
  const [tocfl, setTocfl] = useState("")
  const [exp_group, setExpGroup] = useState("")
  const [Fileidx, setFileidx] = useState("")
  const [cleared, setCleared] = useState(false)
  const [displayImage, setDisplayImage] = useState(null)
  const [showed_text, setshowed_text] = useState("")
  const [showed_posterior, setshowed_posterior] = useState(Array(exp_file_list.length).fill(""))
  const [showed_pinyin, setshowed_pinyin] = useState("")
  const [wavePlayer_url, setwavePlayer_url] = useState("")
  const [boundaries, setboundaries] = useState("")
  const [loaded, setLoaded] = useState(false)
  const [reading_times, setReading_Times] = useState(Array(exp_file_list.length).fill(0))
  const [record_enable, setRecord_Enable] = useState(true)
  const [next_enable, setNext_Enable] = useState(true)
  const [saved_regions, setSaved_Regions] = useState(Array(exp_file_list.length).fill([]))
  const [saved_pitchs, setSaved_Pitchs] = useState(Array(exp_file_list.length).fill([]))
  const [uuid, setUUID] = useState("")

  useEffect(() => {
    const saved = localStorage.getItem("tpt-state")
    if (saved) {
      const state = JSON.parse(saved)
      setParticipant_Id(state.participant_id || "")
      setAge(state.age || "")
      setGender(state.gender || "")
      setMother_Language(state.mother_language || "")
      setStudy_Years(state.study_years || "")
      setTocfl(state.tocfl || "")
      setExpGroup(state.exp_group || "")
      setFileidx(state.Fileidx || 0)
      setCleared(state.cleared || false)
      setDisplayImage(state.displayImage || null)
      setshowed_text(state.showed_text || "")
      setshowed_posterior(state.showed_posterior || Array(exp_file_list.length).fill(""))
      setshowed_pinyin(state.showed_pinyin || "")
      setwavePlayer_url(state.wavePlayer_url || "")
      setReading_Times(state.reading_times || Array(exp_file_list.length).fill(0))
      setRecord_Enable(state.record_enable || true)
      setNext_Enable(state.next_enable || true)
      setSaved_Regions(state.saved_regions || Array(exp_file_list.length).fill([]))
      setSaved_Pitchs(state.saved_pitchs || Array(exp_file_list.length).fill([]))
      setUUID(state.uuid || "")
    }
    //websocket.onopen = () => {
    //  send_profile("profile" + participant_id + "_" + age + "_" + gender + "_" + mother_language + "_" + study_years + "_" + tocfl + "_" + exp_group)
    //}
    setLoaded(true)
  }, [])
  useEffect(() => {
    if (!loaded) return
    localStorage.setItem("tpt-state",
      JSON.stringify({
        participant_id, age, gender, mother_language,
        study_years, tocfl, exp_group, Fileidx, cleared,
        displayImage, showed_text, showed_posterior, showed_pinyin,
        wavePlayer_url, reading_times, record_enable, next_enable,
        saved_regions, saved_pitchs, uuid
      })
    )
  }, [
    loaded, participant_id, age, gender, mother_language,
    study_years, tocfl, exp_group, Fileidx, cleared,
    displayImage, showed_text, showed_posterior, showed_pinyin,
    wavePlayer_url, reading_times, record_enable, next_enable,
    saved_regions, saved_pitchs, uuid
  ])

  const renderFormattedText = (text) => {
    if (!text) return null;

    // 按換行符號拆分每一行
    return text.split('\n').map((line, index) => {
      if (!line.trim()) return null; // 略過空行

      let color = "#ff0000"; // 預設顏色
      if (line.startsWith("Ok")) {
        color = "#ffc400";
      } else if (line.startsWith("Good")) {
        color = "#00FF00"; // 亮綠色
      }

      return (
        <div key={index} style={{ color: color, marginBottom: '5px' }}>
          {line.split('|')[1]}
        </div>
      );
    });
  };

  return (
    <div className="App" style={{ background: "#f0f0f0", position: "relative" }}>
      {cleared && displayImage && (
        <>
          <h1 style={{ textAlign: "left", marginTop: "0px", marginLeft: "20px", fontSize: '30px' }}>{showed_text} </h1>
          <h1 style={{ textAlign: "left", marginTop: "-20px", marginLeft: "20px", fontSize: '20px' }}>{showed_pinyin} </h1>
          <div style={{ position: "absolute", top: "10px", right: "10px", width: "15vw", zIndex: 1000, }} >
            <img src={displayImage} style={{ width: "100%", display: "block", }} />
            <div style={{ display: "flex", justifyContent: "space-between", marginTop: "8px" }}>
              <h1 style={{ margin: 0, fontSize: "15px" }}>Fileidx: ({Fileidx}/40)</h1>
              <h1 style={{ margin: 0, fontSize: "15px" }}>reading_times: ({reading_times[Fileidx]}/20)</h1>
            </div>
            <h1 style={{ margin: "20px 0 0 0", textAlign: "left", fontSize: '18px', whiteSpace: "pre-line" }}>{renderFormattedText(showed_posterior[Fileidx])} </h1>
          </div>

          <input type="text" placeholder="Go to" onKeyDown={(e) => { if (e.key === "Enter") { handlegotoidx(e.target.value, setFileidx, setwavePlayer_url, setshowed_text, setshowed_pinyin) } }} style={{ width: "3em", position: "absolute", left: "75px" }} />
          <button style={{ position: "absolute", left: "0px" }} onClick={() => handlerestart(setParticipant_Id, setAge, setGender, setMother_Language, setStudy_Years, setTocfl, setExpGroup, setCleared, setFileidx, setshowed_posterior, setReading_Times, setSaved_Regions, setSaved_Pitchs, setRecord_Enable, setNext_Enable)}> restart </button>
          <div style={{ display: "flex", alignItems: "center", justifyContent: "center", gap: "10px" }}>
            <Self_Evaluation Fileidx={Fileidx} reading_times={reading_times[Fileidx]} setNext_Enable={setNext_Enable} setRecord_Enable={setRecord_Enable} />
            <button disabled={!next_enable} onClick={() => handlenextwav(setFileidx, Fileidx, setwavePlayer_url, setshowed_text, setshowed_pinyin)}> Next </button>
          </div>


          < WavePlayer_example url={wavePlayer_url} EG={exp_group} />
          <button onClick={() => recompute(boundaries, setboundaries, saved_regions, setSaved_Regions, Fileidx, reading_times[Fileidx])}> recompute </button>
          < Retrecordhtmls key={Fileidx} tag="exp" Fileidx={Fileidx} exp_group={exp_group} profile={participant_id + "_" + age + "_" + gender + "_" + mother_language + "_" + study_years + "_" + tocfl} setshowed_posterior={setshowed_posterior} setboundaries={setboundaries} reading_times={reading_times} setReading_Times={setReading_Times} record_enable={record_enable} saved_regions={saved_regions} setSaved_Regions={setSaved_Regions} saved_pitchs={saved_pitchs} setSaved_Pitchs={setSaved_Pitchs} setUUID={setUUID} uuid={uuid} setRecord_Enable={setRecord_Enable} setNext_Enable={setNext_Enable}/>
        </>
      )}

      {!cleared && (
        <>
          <div>
            <h1>Welcome</h1>
            < Welcomeinterface setPI={setParticipant_Id} setA={setAge} setG={setGender} setM={setMother_Language} setS={setStudy_Years} setT={setTocfl} setEG={setExpGroup} />
            < ConfirmSubjectInfo PI={participant_id} A={age} G={gender} M={mother_language} S={study_years} T={tocfl} EG={exp_group} setCleared={setCleared} setDisplayImage={setDisplayImage} setwavePlayer_url={setwavePlayer_url} setshowed_text={setshowed_text} Fileidx={Fileidx} setshowed_pinyin={setshowed_pinyin} setReading_Times={setReading_Times} setUUID={setUUID} />

          </div>
          <h1>Nice react audio player with Wavesurfer.js + Timeline </h1>
          < WavePlayer url="./exp_data/mono_example_fan1.wav" />
          < WavePlayer url="./exp_data/mono_example_wu2.wav" />
          < WavePlayer url="./exp_data/mono_example_kou3.wav" />
          < WavePlayer url="./exp_data/mono_example_chu4.wav" />
          < WavePlayer url="./exp_data/sentence_example_1.wav" />
          { /*< Retrecordhtmls tag="demo" Fileidx="" exp_group="" profile="" setReading_Times={setReading_Times} setUUID={setUUID} /> */}
        </>
      )
      }
    </div>
  )
}
//< Recording />
//disabled={!allFilled}
//      < WavePlayer url="./exp_data/mono_example_fan1.wav" />
//      < WavePlayer url="./exp_data/mono_example_wu2.wav" />
//      < WavePlayer url="./exp_data/mono_example_kou3.wav" />
//      < WavePlayer url="./exp_data/mono_example_chu4.wav" />

//<img src={displayImage} style={{ position: "absolute", top: "10px", right: "10px", width: "15vw", zIndex: 1000 }} />
//<h1 style={{ position: "absolute", right: "0px", textAlign: "right",marginTop: "100px", marginLeft: "0px", fontSize: '15px', whiteSpace: "pre-line"}}>reading_times: ({reading_times}/20) </h1>
//<h1 style={{ position: "absolute", right: "0px", textAlign: "right",marginTop: "150px", marginLeft: "0px", fontSize: '20px', whiteSpace: "pre-line"}}>{showed_posterior} </h1>