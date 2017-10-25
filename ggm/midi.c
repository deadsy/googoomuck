//-----------------------------------------------------------------------------
/*

MIDI Functions

*/
//-----------------------------------------------------------------------------

#include <string.h>
#include <math.h>

#include "ggm_internal.h"

//-----------------------------------------------------------------------------

#define NOTES_IN_OCTAVE 12

//-----------------------------------------------------------------------------
// note to name conversion

static const char *sharps[NOTES_IN_OCTAVE] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
static const char *flats[NOTES_IN_OCTAVE] = { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };

// convert a midi note to a name
const char *midi_note_name(uint8_t note, char mode) {
	note %= NOTES_IN_OCTAVE;
	return (mode == '#') ? sharps[note] : flats[note];
}

// return a note name with sharp and flat forms
char *midi_full_note_name(char *str, uint8_t note) {
	const char *s_name = midi_note_name(note, '#');
	const char *f_name = midi_note_name(note, 'b');
	strcpy(str, s_name);
	if (strcmp(s_name, f_name)) {
		strcat(str, "/");
		strcat(str, f_name);
	}
	return str;
}

//-----------------------------------------------------------------------------

// return an octave number
int midi_to_octave(uint8_t note) {
	return (note / NOTES_IN_OCTAVE);
}

// return the frequency of the midi note
float midi_to_frequency(uint8_t note) {
	return 440.0f * powf(2.0f, (float)(note - 69) / 12.0f);
}

//-----------------------------------------------------------------------------
