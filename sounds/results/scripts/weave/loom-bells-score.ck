//-----------------------------------------------------------------------------
// name: loom-bells.ck
// desc: bell section
//       loom.ck should be executed before running this
//-----------------------------------------------------------------------------

// midi device number
0 => int device;

2.0::second => dur T;
T - (now % T) => now;

4.0 /=> T;

// which bus
3 => int bus;
// allocate taps template
TapsTemp bells[8];
TapsTemp @ bell;

// copy
TapsSynth.copy( "lutinebell1", 0, bells.cap()-1 );
// read from library
Loom.readFromLibrary( bells, "lutinebell1" );

// scale ratios
float s[12];
for( int i; i < s.cap(); i++ )
    std.mtof( i + 60 ) / std.mtof( 60 ) => s[i];

// scale
// 0  1  2  3  4  5  6  7
[  0, 2, 4, 5, 7, 8, 9, 11 ] @=> int scale[];
// silence
-1 => int X;
// quarter note
4 => int Q;

[ // score

  [2,Q], [2,Q], [1,Q], [2,Q],
  [2,Q], [2,Q], [1,Q], [4,Q],
  [X,Q], [4,Q], [1,Q], [2,Q],
  [2,Q], [2,Q], [1,Q], [2,Q]

] @=> int score[][];

// device to open
MidiIn min;
MidiMsg msg;

// open
if( !min.open( device ) ) me.exit();

// parameters
.5 => float pan;
.25 => float pan_range;
.05 => float time_stretch;
.1 => float time_stretch_range;
.05 => float prob;
0 => int range;
3 => float factor;
4 => float t;
int which;
int where;

// wait for midi
spork ~ midi();

// reverb
Loom.busReverbTo( bus, .01 );

// infinite time loop
while( true )
{
    // get
    get() @=> bell;

    // gain
    std.rand2f( .5, 1 ) => bell.gain;
    // pan
    std.rand2f( pan-pan_range, pan+pan_range ) => bell.pan;
    // time stretch
    std.rand2f( time_stretch-(time_stretch*time_stretch_range), 
                time_stretch+(time_stretch*time_stretch_range) ) => float v;
    if( v <= .001 ) .001 => v;
    v => bell.timeStretch;
    // freq
    score[where][0] => int winner;
    if( winner >= 0 )
    {
        math.pow( 2, std.rand2(0,range) ) * s[scale[winner]] / factor => bell.freqWarp;
        // play
        bell.play( bus );
    }

    // advance time
    T/score[where][1] => now;

    where++;
    score.cap() %=> where;
}

// polyfony
fun TapsTemp get()
{
    which++;
    bells.cap() %=> which;
    return bells[which];
}

// midi
fun void midi()
{
    // infinite time loop
    while( true )
    {
        // wait on event
        min => now;

        // get messages
        while( min.recv( msg ) )
        {
            // knob
            if( msg.data1 == 176 )
            {
                // pan
                if( msg.data2 == 11 )
                    <<< "pan:", msg.data3 / 127.0 => pan>>>;
                else if( msg.data2 == 12 )
                    <<< "pan range", msg.data3 / 254.0 => pan_range>>>;
                else if( msg.data2 == 13 )
                    Loom.busReverbTo( bus, msg.data3/508.0 );
                else if( msg.data2 == 14 )
                    <<< "prob:", msg.data3 / 127.0 => prob>>>;
                else if( msg.data2 == 15 )
                    <<< "time:", .01 + msg.data3 / 508.0 => time_stretch>>>;
                else if( msg.data2 == 16 )
                    <<< "time range:", msg.data3 / 127.0 => time_stretch_range>>>;
                else if( msg.data2 == 17 )
                    msg.data3 / 16 => range;
                else if( msg.data2 == 18 )
                    <<< "factor:", 12 - (msg.data3 / 127.0 * 11.0) => factor>>>;
                else if( msg.data2 == 1 )
                    <<< "speed:", .5 + msg.data3 / 127.0 * 16 => t>>>;
            }
        }
    }
}