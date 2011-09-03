//------------------------------------------------
// name: loom-midi-bells.ck
// desc: run loom.ck before this
//--------------------------------------------

// device to open
0 => int device;

MidiIn min;
MidiMsg msg;

// try to open MIDI port (see chuck --probe for available devices)
if( !min.open( device ) ) me.exit();

// print out device that was opened
<<< "MIDI device:", min.num(), " -> ", min.name() >>>;

// make our own event
class NoteEvent extends Event
{
    int note;
    int velocity;
}

// the event
NoteEvent on;
// array of ugen's handling each note
Event @ us[128];

// bell
TapsTemp bells[10];
// copy
TapsSynth.copy( "lutinebell1", 0, bells.cap()-1 );
// read from library
Loom.readFromLibrary( bells, "lutinebell1" );

// bus
0 => int bus;
// reverb
Loom.busReverbTo( bus, .1 );

// better low end resolution
1000 => float base;
.01 => float slide_0;
100 => float slide_1;
(slide_1 - slide_0) / (base-1) => float k;
slide_0 - k => float c;

// stretch
1.0 => float stretch;
1.0 => float gain;

// handler shred for a single voice
fun void handler( int which )
{
    Event off;
    int note;

    // inifinite time loop
    while( true )
    {
        on => now;
        on.note => note;
        std.mtof( note ) / std.mtof(60) => bells[which].freqWarp;
        gain * on.velocity / 128.0 => bells[which].gain;
        bells[which].play( bus ); 
        off @=> us[note];

        off => now;
        null @=> us[note];
        100::ms => now;
    }
}

// spork handlers, one for each voice
for( 0 => int i; i < bells.cap(); i++ ) spork ~ handler( i );

// infinite time loop
while( true )
{
    // wait on midi event
    min => now;

    // get the midimsg
    while( min.recv( msg ) )
    {
        // 176
        if( msg.data1 == 176 )
        {
            // volume
            if( msg.data2 == 12 )
            {
                for( int i; i < bells.cap(); i++ )
                {
                    bells[i].gain( .001 + msg.data3/128.0 => gain );
                }
            }
            // reverb
            if( msg.data2 == 13 )
            {
                Loom.busReverbTo( bus, msg.data3/512.0 );
            }
            // time stretch
            if( msg.data2 == 17 )
            {
                for( int i; i < bells.cap(); i++ )
                {
                    bells[i].timeStretch( k * math.pow(base,msg.data3/128.0) + c => stretch );
                }
            }
        }

        // note on
        if( msg.data1 == 144 )
        {
            if( msg.data3 > 0 )
            {
                // store midi note number
                msg.data2 => on.note;
                // store velocity
                msg.data3 => on.velocity;
                // signal the event
                on.signal();
                // yield without advancing time to allow shred to run
                me.yield();
            }
            else
            {
                if( us[msg.data2] != null ) us[msg.data2].signal();
            }
        }
    }
}
