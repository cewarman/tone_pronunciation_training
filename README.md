# Chinese Tone Training Web Service

A web-based pronunciation training platform for Mandarin Chinese tones. The system helps learners improve their tone production by allowing them to compare their own pitch contour with a reference example.

Users can listen to native-speaker recordings, observe the corresponding pitch contour, and then record their own pronunciation. The platform visualizes both contours side by side so that learners can imitate, adjust, and gradually improve their tone accuracy.

---

## Features

- Play reference audio for Chinese syllables, words, or sentences
- Display the pitch contour of the reference pronunciation
- Record the learner's own pronunciation directly in the browser
- Extract and visualize the learner's pitch contour
- Compare the learner's contour with the reference contour
- Support repeated practice and immediate visual feedback
- Web-based interface with no additional software required

---

## Motivation

Mandarin Chinese is a tonal language, and incorrect tone production can change the meaning of a word completely. Many learners have difficulty hearing and reproducing the differences between tones.

This project aims to make tone learning more intuitive by transforming speech into visual pitch contours. By seeing how their pronunciation differs from the target contour, learners can more easily identify problems and make corrections.

---

## How It Works

1. The user selects a training item.
2. The system plays a reference pronunciation.
3. The reference pitch contour is displayed.
4. The user records their own pronunciation.
5. The system extracts the user's pitch contour.
6. Both contours are shown together for comparison.
7. The user repeats the process until the pronunciation matches the target more closely.

---

## Example Use Cases

- Practicing the four Mandarin tones
- Distinguishing commonly confused tone pairs
- Improving pronunciation in words and sentences
- Self-study for second-language learners
- Classroom support for Chinese language instruction

---

## Technologies

Possible technologies used in this project include:

- Frontend: React / JavaScript / HTML / CSS
- Audio processing: Web Audio API, WaveSurfer.js
- Pitch extraction: Python, Librosa, Praat, or other signal processing tools
- Backend: Python, FastAPI, Flask, or similar frameworks
- Database: PostgreSQL or other storage systems

---

## Future Improvements

- Automatic tone classification
- Pronunciation scoring
- Personalized learning recommendations
- Support for more Chinese dialects and languages
- Mobile-friendly interface
- Real-time feedback during speaking

---

## License

This project is released under the MIT License.

