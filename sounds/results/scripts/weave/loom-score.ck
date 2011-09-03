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
// duck loop
TapsTemp duckloop;
// read from library
Loom.readFromLibrary( duckloop, "quack-loop" );

// horn
TapsTemp horn;
// read from library
Loom.readFromLibrary( horn, "italyhorn" );

// bell
TapsTemp bells[4];
// copy
TapsSynth.copy( "lutinebell1", 0, bells.cap()-1 );
// read from library
Loom.readFromLibrary( bells, "lutinebell1" );
// bell loop
TapsTemp bellloop;
// read from library
Loom.readFromLibrary( bellloop, "lutinebell1-loop" );

// squawk
TapsTemp squawk;
// read from library
Loom.readFromLibrary( squawk, "squawk" );

// baby
TapsTemp baby;
// read from library
Loom.readFromLibrary( baby, "baby" );

// children
TapsTemp children[10];
// read from Library
Loom.readFromLibrary( children[0], "child" );
Loom.readFromLibrary( children[1], "child1" );
Loom.readFromLibrary( children[2], "child2" );
Loom.readFromLibrary( children[3], "child3" );
Loom.readFromLibrary( children[4], "child4" );
Loom.readFromLibrary( children[5], "child5" );
Loom.readFromLibrary( children[6], "child6" );
Loom.readFromLibrary( children[7], "child7" );
Loom.readFromLibrary( children[8], "child8" );
Loom.readFromLibrary( children[9], "child9" );

// glass
TapsTemp glass;
// read from Library
Loom.readFromLibrary( glass, "glassbreak" );

// orginal sounds
sndbuf oceanL;
sndbuf oceanR;
// load
"data/waves.wav" => oceanL.read;
"data/waves.wav" => oceanR.read;
// select channel
oceanL.channel(0);
oceanR.channel(1);

// play
fun void ocean()
{
    // patch
    oceanL => dac.left;
    oceanR => dac.right;

    1 => oceanL.gain => oceanR.gain;
    0 => oceanL.pos => oceanR.pos;
    <<< oceanL.length() / second, oceanL.rate() >>>;
    oceanL.length() => now;

    // unpatch
    oceanL =< dac.left;
    oceanR =< dac.right;
}


//-----------------------------------------------------------------------------
// BEGINNING OF PIECE
//-----------------------------------------------------------------------------

// start timer
spork ~ Loom.timer( now );

//-----------------------------------------------------------------------------
// SECTION 1: OCEAN
//-----------------------------------------------------------------------------

fun void section_one()
{
    // play ocean
    spork ~ ocean();

    // set loop parameters;
    Loom.randomTo( birdloop, 2.0 );
    Loom.periodicityTo( birdloop, .25 );
    Loom.densityTo( birdloop, .5 );
    Loom.freqTo( birdloop, 1.0 );
    Loom.stretchTo( birdloop, .75 );
    Loom.gainTo( birdloop, 0 );
    Loom.panTo( birdloop, .5 );

    // start it
    birdloop.play( bus_bird );
    // ramp gain
    Loom.gainTo( birdloop, .5, 5::second );

    // play with freq and time and density
    spork ~ Loom.freqTo( birdloop, 1.5, 7.5::second );
    spork ~ Loom.densityTo( birdloop, 5.0, 7.5::second );
    spork ~ Loom.gainTo( birdloop, .5, 7.5::second );
    Loom.stretchTo( birdloop, .5, 5::second );
    Loom.stretchTo( birdloop, 1.5, 5::second );

    // advance time
    2::second => now;

    // bring down density
    // spork ~ Loom.densityTo( birdloop, .5, 5::second );

    // advance time
    3::second => now;

    // bring freq, time down
    spork ~ Loom.freqTo( birdloop, .5, 5::second );
    Loom.stretchTo( birdloop, .75, 3::second );

    // bring up
    spork ~ Loom.randomTo( birdloop, 2.5, 3::second );
    Loom.stretchTo( birdloop, 1.0, 5::second );

    // stop it
    // birdloop.stop();

    // wait
    5::second => now;

    // restart
    // birdloop.play( bus_bird );

    // bring down density
    spork ~ Loom.densityTo( birdloop, .5, 5::second );

    5::second => now;

    // bring up gain, bring up freq
    spork ~ Loom.gainTo( birdloop, .7, 8::second );
    spork ~ Loom.freqTo( birdloop, .85, 8::second );

    12::second => now;
}

//-----------------------------------------------------------------------------
// SECTION 2: RAIN FOREST
//-----------------------------------------------------------------------------

fun void section_two()
{
    // play bird loop
    if( birdloop.playing() == false )
        birdloop.play( bus_bird );

    // bring up reverb
    Loom.busReverbTo( bus_bird, .1, 5::second );

    // let it ring, pan bird
    Loom.panTo( birdloop, .75, 10::second );

    // configure duck
    Loom.randomTo( duckloop, 2.0 );
    Loom.periodicityTo( duckloop, .25 );
    Loom.densityTo( duckloop, .5 );
    Loom.freqTo( duckloop, 1.0 );
    Loom.stretchTo( duckloop, 1.0 );
    Loom.gainTo( duckloop, 0 );
    Loom.panTo( duckloop, .25 );

    // bring in duck
    duckloop.play( bus_bird );
    spork ~ Loom.gainTo( duckloop, .75, 8::second );
    // wait
    6::second => now;

    // up density
    spork ~ Loom.densityTo( birdloop, 2.0, 5::second );
    5::second => now;

    // up duck
    spork ~ Loom.densityTo( duckloop, 2.0, 5::second );
    spork ~ Loom.periodicityTo( duckloop, 0, 5::second );
    spork ~ Loom.freqTo( duckloop, 1.25, 3::second );

    // bring down bird gain
    spork ~ Loom.gainTo( birdloop, .15, 10::second );

    // bring up duck density
    spork ~ Loom.densityTo( duckloop, 5, 10::second );
    // down duck stretch
    spork ~ Loom.stretchTo( duckloop, .55, 5::second );
    // wait
    5::second => now;

    // down duck frequency
    Loom.freqTo( duckloop, .25, 5::second );
    // let ring
    5::second => now;
    // down duck frequency
    spork ~ Loom.freqTo( duckloop, .06, 10::second );
    // up duck stretch
    spork ~ Loom.stretchTo( duckloop, 4, 10::second );
    // wait
    3::second => now;
    // bird frequency
    spork ~ Loom.freqTo( birdloop, 1, 5::second );
    spork ~ Loom.gainTo( birdloop, .05, 10::second );

    // let ring
    10::second => now;

    // take out ducks
    spork ~ Loom.gainTo( duckloop, .0001, 10::second );
    // make birds periodicity
    spork ~Loom.gainTo( birdloop, .65, 10::second );
    Loom.periodicityTo( birdloop, 1, 10::second );
    Loom.randomTo( birdloop, 1, 5::second );

    4::second => now;

    // get ready for tone
    spork ~ Loom.freqTo( birdloop, .25, 15::second );
    spork ~ Loom.stretchTo( birdloop, .025, 15::second );
    spork ~ Loom.densityTo( birdloop, 30, 15::second );
    spork ~ Loom.panTo( birdloop, .5, 15::second );

    15::second => now;
}


//-----------------------------------------------------------------------------
// SECTION THREE: bird tone
//-----------------------------------------------------------------------------
fun void section_three()
{
    // play
    if( birdloop.playing() == false )
    {
        birdloop.play( bus_bird );

        spork ~ Loom.gainTo( birdloop, .65, 10::second );
        spork ~ Loom.periodicityTo( birdloop, 1, 10::second );
        spork ~ Loom.randomTo( birdloop, 1, 5::second );
        10::second => now;
        spork ~ Loom.freqTo( birdloop, .25, 15::second );
        spork ~ Loom.stretchTo( birdloop, .025, 15::second );
        spork ~ Loom.densityTo( birdloop, 30, 15::second );
        spork ~ Loom.panTo( birdloop, .5, 15::second );
        15::second => now;
    }

    // freq up/down
    Loom.freqTo( birdloop, .5, 3::second );
    Loom.freqTo( birdloop, .25, 3::second );
    // density down/up
    Loom.densityTo( birdloop, 8, 3::second );
    Loom.densityTo( birdloop, 30, 3::second );

    // freq up/down
    Loom.freqTo( birdloop, .75, 4::second );
    Loom.freqTo( birdloop, .25, 2::second );
    // density down/up
    Loom.densityTo( birdloop, 6, 5::second );
    Loom.densityTo( birdloop, 30, 3::second );

    // time down
    Loom.stretchTo( birdloop, .025, 5::second );

    // up down
    birdboom( birdloop, .75, 2, 10, .5, 2.5, 0, 5::second, 3::second, 10::second );

    // up down
    birdboom( birdloop, .9, 1.0, 15, .5, 2.5, 0, 8::second, 4::second, 12::second );

    // gain down
    spork ~ Loom.freqTo( birdloop, 1, 10::second );
    Loom.gainTo( birdloop, .001, 8::second );

    // stop
    birdloop.stop();
}


//-----------------------------------------------------------------------------
// SECTION FOUR: bells
//-----------------------------------------------------------------------------
fun void section_four()
{
    // reverb
    Loom.busReverbTo( bus_glass, .1, 1::second );
    Loom.busReverbTo( bus_bell, .1, 1::second );

    // normal
    Loom.freqTo( squawk, 1 );
    Loom.stretchTo( squawk, 1 );

    // squawk
    squawk.play( bus_glass );
    
    // wait for
    Loom.wait( squawk );
    // wait a bit more
    2::second => now;

    // play
    Loom.freqTo( squawk, .5 );
    squawk.play( bus_glass );

    // transform
    spork ~ Loom.freqTo( squawk, 1.122, 10::second );
    spork ~ Loom.stretchTo( squawk, 8.414, 10::second );
    
    // wait
    Loom.wait( squawk );
    // wait a bit more
    5::second => now;

    // play
    squawk.play( bus_glass );

    // wait
    // Loom.wait( squawk );
    5::second => now;
    spork ~ Loom.gainTo( squawk, 0.0001, 2::second );
    // wait a bit more
    3::second => now;


    // bells
    bells[0].freqWarp( .25 );
    bells[1].freqWarp( .5 );
    bells[2].freqWarp( 1 );
    bells[3].freqWarp( 1.5 );

    // reverb
    spork ~ Loom.busReverbTo( bus_glass, 0 );
    spork ~ Loom.busReverbTo( bus_bell, .15, 1::second );

    // play
    bells[2].play( bus_bell );
    bellstretch( bells[2], 1, .25::second, 2, 3::second );
    // wait
    Loom.wait( bells[2] );
    // wait a bit more
    1::second => now;

    // play
    spork ~ Loom.gainTo( bells[2], 1.5, 3::second );
    bells[2].play( bus_bell );
    spork ~ bellstretch( bells[2], 1, .25::second, 4, 5::second );
    bellwarp( bells[2], 1, 3::second, .05, 10::second );
    // wait
    Loom.wait( bells[2] );

    // play
    bells[2].play( bus_bell );
    spork ~ Loom.gainTo( bells[2], 2.5, 5::second );
    spork ~ bellstretch( bells[2], 1, .25::second, 5, 3::second );
    bellwarp( bells[2], .05, .5::second, 1, 10::second );
    // wait
    // Loom.wait( bells[2] );
    5::second => now;

    spork ~ Loom.gainTo( bells[2], 1.5, 5::second );

    {
        // play
        bells[0].play( bus_bell );
        spork ~ bellstretch( bells[0], 1, .25::second, 5, 3::second );
        spork ~ bellwarp( bells[0], 1.5, 1::second, .5, 10::second );
        // wait
        std.rand2f(5,8)::second => now;

        // play
        bells[1].play( bus_bell );
        spork ~ bellstretch( bells[1], 1, .25::second, 5, 3::second );
        spork ~ bellwarp( bells[1], .75, .5::second, .5, 10::second );
        // wait
        std.rand2f(5,8)::second => now;

        // play
        bells[2].play( bus_bell );
        spork ~ bellstretch( bells[2], 1, .25::second, 5, 3::second );
        spork ~ bellwarp( bells[2], .75, .5::second, 1.5, 10::second );
        // wait
        std.rand2f(5,8)::second => now;

        // play
        bells[3].play( bus_bell );
        spork ~ bellstretch( bells[3], 1, .25::second, 5, 3::second );
        spork ~ bellwarp( bells[3], .25, .5::second, 1, 10::second );
        // wait
        std.rand2f(3,7)::second => now;
    }

    {
        // play
        bells[0].play( bus_bell );
        spork ~ bellstretch( bells[0], 1, .25::second, 5, 3::second );
        spork ~ bellwarp( bells[0], 1.5, 1::second, .5, 10::second );
        // wait
        std.rand2f(2,3)::second => now;

        // squawk
        spork ~ gosquawk();
        // wait
        std.rand2f(2,3)::second => now;

        // play
        bells[2].play( bus_bell );
        spork ~ bellstretch( bells[2], 1, .25::second, 5, 3::second );
        spork ~ bellwarp( bells[2], .75, .5::second, 1.5, 10::second );
        // wait
        std.rand2f(5,8)::second => now;

        // play
        bells[1].play( bus_bell );
        spork ~ bellstretch( bells[1], 1, .25::second, 5, 3::second );
        spork ~ bellwarp( bells[1], .75, .5::second, .5, 10::second );
        // wait
        std.rand2f(5,8)::second => now;

        // play
        bells[3].play( bus_bell );
        spork ~ bellstretch( bells[3], 1, .25::second, 5, 3::second );
        spork ~ bellwarp( bells[3], .25, .5::second, 1, 10::second );
        // wait
        std.rand2f(3,7)::second => now;
    }

    Loom.busReverbTo( bus_bell, 0, 1::second );
}

fun void section_five()
{
    // reverb
    Loom.busReverbTo( bus_children, .1 );

    // child7
    Loom.stretchTo( children[7], 50 );
    Loom.freqTo( children[7], .1 );

    // child9
    Loom.stretchTo( children[9], 50 );
    Loom.freqTo( children[9], .1 );

    // play
    children[7].play( bus_children );
    spork ~ Loom.freqTo( children[7], .03, 10::second );
    spork ~ Loom.gainTo( children[7], 2.5, 15::second );
    // wait
    5::second => now;

    // modulate
    float t;
    now + 1::minute => time later;

    while( now < later )
    {
        .005 * math.sin(t) + .03 => children[7].freqWarp;
        5::ms => now;
        .01 +=> t;
    }

    <<< "done modulate...", "" >>>;
}

fun void section_six()
{
    
}

section_one();
section_two();
section_three();
section_four();
section_five();

// end
<<< "ending..." >>>;

// gosquawk
fun void gosquawk()
{
    // turn on reverb
    Loom.busReverbTo( bus_glass, .1, 2::second );
    // gain
    Loom.gainTo( squawk, 1, .5::second );
    // play
    squawk.play( bus_glass );
    // wait
    Loom.wait( squawk );
    // wait a bit more
    2::second => now;
    // turn off reverb
    Loom.busReverbTo( bus_glass, 0 );
}

// play bell
fun void bellstretch( TapsTemp t, float timeA, dur wait, float timeB, dur T )
{
    // set stretch
    t.timeStretch( timeA );
    // play
    // t.play( bus_bell );
    // wait
    wait => now;
    // time
    Loom.stretchTo( t, timeB, T );
}

// play bell
fun void bellwarp( TapsTemp t, float freqA, dur wait, float freqB, dur T )
{
    // set freq
    t.freqWarp( freqA );
    // play
    // t.play( bus_bell );
    // wait
    wait => now;
    // time
    Loom.freqTo( t, freqB, T );
}

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
