//-----------------------------------------------------------------------------
// name: loom-score.ck
// desc: the piece
//       loom.ck should be executed before running this
//-----------------------------------------------------------------------------

// initial settings on all buses, volume, reverb, pan
Loom.allSet( 1.0, 0.0, 0.0 );

// scale ratios
float s[12];
for( int i; i < s.cap(); i++ )
    std.mtof( i + 60 ) / std.mtof( 60 ) => s[i];

// bus assignment
0 => int bus_bird;
1 => int bus_horn;
2 => int bus_synth;
3 => int bus_bell;
4 => int bus_baby;
5 => int bus_children;
6 => int bus_glass;
7 => int bus_chuck;

// bird
TapsTemp bird;
// read from library
Loom.readFromLibrary( bird, "chirp1" );
// bird loop
TapsTemp birdloop;
// read from library
Loom.readFromLibrary( birdloop, "chirp1loop" );


//-----------------------------------------------------------------------------
// BEGINNING OF PIECE
//-----------------------------------------------------------------------------

// start timer
spork ~ Loom.timer( now );

// go
flock();

//-----------------------------------------------------------------------------
// flock
//-----------------------------------------------------------------------------
fun void flock()
{
    birdloop.play( bus_bird );

    spork ~ Loom.gainTo( birdloop, .65, 6::second );
    spork ~ Loom.periodicityTo( birdloop, 1, 6::second );
    spork ~ Loom.randomTo( birdloop, 1, 5::second );
    6::second => now;
    spork ~ Loom.freqTo( birdloop, .2, 15::second );
    spork ~ Loom.stretchTo( birdloop, .05, 15::second );
    spork ~ Loom.densityTo( birdloop, 30, 15::second );
    spork ~ Loom.panTo( birdloop, .5, 15::second );
    15::second => now;

    // freq up/down
    Loom.freqTo( birdloop, .75, 3::second );
    Loom.freqTo( birdloop, .15, 2::second );
    // density down/up
    Loom.densityTo( birdloop, 9, 4::second );
    1::second => now;
    Loom.densityTo( birdloop, 30, 2::second );

    // up down
    birdboom( birdloop, .75, 2, 10, .5, 2.5, 0, 5::second, 3::second, 5::second );

    // up down
    birdboom( birdloop, .9, 1.0, 15, .5, 2.5, 0, 3::second, 3::second, 3::second );

    // gain down
    spork ~ Loom.freqTo( birdloop, 1, 4::second );
    Loom.gainTo( birdloop, .1, 4::second );

    // stop
    birdloop.stop();
}

// end
<<< "ending..." >>>;

// bird play
fun void birdboom( TapsTemp loop, float freq, float time, float density, float gain, float random, float period, dur Tup, dur Tmiddle, dur Tdown )
{
    // save settings
    loop.freqWarp() => float old_freq;
    loop.timeStretch() => float old_time;
    loop.density() => float old_density;
    loop.gain() => float old_gain;
    loop.random() => float old_random;
    loop.periodicity() => float old_period;

    // freq up
    spork ~ Loom.freqTo( loop, freq, Tup );
    // time up
    spork ~ Loom.stretchTo( loop, time, Tup );
    // density down
    spork ~ Loom.densityTo( loop, density, Tup );
    // gain down
    spork ~ Loom.gainTo( loop, gain, Tup );
    // random up
    spork ~ Loom.randomTo( loop, random, Tup );
    // periodicity down
    spork ~ Loom.periodicityTo( loop, period, Tup );

    // wait
    Tmiddle => now;

    // freq down
    spork ~ Loom.freqTo( loop, old_freq, Tdown );
    // time down
    spork ~ Loom.stretchTo( loop, old_time, Tdown );
    // density up
    spork ~ Loom.densityTo( loop, old_density, Tdown );
    // gain up
    spork ~ Loom.gainTo( loop, old_gain, Tdown );
    // periodicity down
    spork ~ Loom.periodicityTo( loop, old_period, Tdown );
    // random down
    spork ~ Loom.randomTo( birdloop, old_random, Tdown );

    // wait
    Tdown => now;
}
