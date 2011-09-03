
// initialize
8 => Loom.num_buses;

// initial settings
Loom.allSet( 1.0, 0.0, 0.0 );

// load templates into synthesis face

// chirp1
Loom.load( "chirp1" );

// chirp1-loops
Loom.load( "chirp1loop" );

// italy horn
Loom.load( "italyhorn" );

// lutinebell1
Loom.load( "lutinebell1" );

// children
Loom.load( "child1" );
Loom.load( "child2" );
Loom.load( "child3" );
Loom.load( "child4" );
Loom.load( "child5" );
Loom.load( "child6" );
Loom.load( "child7" );
Loom.load( "child8" );
Loom.load( "child9" );

// quacks and squawks
Loom.load( "quack" );
// quack-loop
Loom.load( "quack-loop" );
Loom.load( "quack4" );
Loom.load( "squawk" );

// class Loom
public class Loom
{
    static int num_buses;

    fun static void load( string name, int copies )
    {
        if( !TapsSynth.load( name ) )
        {
            <<< "load.ck: cannot load", name >>>;
            me.exit();
        }

        if( copies > 1 )
        {
            TapsSynth.copy( name, 0, copies - 1 );
        }
    }

    fun static void load( string name )
    {
        load( name, 1 );
    }

    fun static void readFromLibrary( TapsTemp temps[], string name )
    {
        for( int i; i < temps.cap(); i++ )
        {
            // load it from library
            temps[i].readFromLibrary( name, i );
            // make sure it loaded
            if( !temps[i].ready() )
            {
                <<< "cannot load:", name, "number:", i >>>;
                me.exit();
            }
        }
    }

    fun static void readFromLibrary( TapsTemp temp, string name, int which )
    {
        // load
        temp.readFromLibrary( name, which );
        // make sure it loaded
        if( !temp.ready() )
        {
            <<< "cannot load:", name, which >>>;
            me.exit();
        }
    }

    fun static void readFromLibrary( TapsTemp temp, string name )
    {
        readFromLibrary( temp, name, 0 );
    }

    fun static void busVolume( int which, float value )
    { TapsBus.volume( which, value ); }
    fun static void busReverb( int which, float value )
    { TapsBus.reverb( which, value ); }
    fun static void busPan( int which, float value )
    { TapsBus.reverb( which, value ); }

    fun static void allVolume( float value )
    {
        for( int i; i < num_buses; i++ )
            busVolume( i, value );
    }

    fun static void allReverb( float value )
    {
        for( int i; i < num_buses; i++ )
            busReverb( i, value );
    }

    fun static void allPan( float value )
    {
        for( int i; i < num_buses; i++ )
            busPan( i, value );
    }

    fun static void allSet( float volume, float reverb, float pan )
    {
        allVolume( volume );
        allReverb( reverb );
        allPan( pan );
    }

    fun static void gainTo( TapsTemp temp, float target, dur T, dur inc )
    {
        if( T > 0::second )
        {
            temp.gain() => float curr;
            T / inc => float steps;
            target - curr => float diff;
            diff / steps => float val;
            float iter;

            <<< "setting volume to:", target, "in", T/ms, "ms, inc:", inc/ms, "ms" >>>;
            while( iter < steps )
            {
                curr + iter * val => temp.gain;
                1 +=> iter;
                inc => now;
            }
        }

        target => temp.gain;
        <<< "volume set to:", target >>>;
    }

    fun static void gainTo( TapsTemp temp[], float target, dur T, dur inc )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ gainTo( temp[i], target, T, inc );
		T => now;
    }

    fun static void gainTo( TapsTemp temp, float target, dur T )
    {
        gainTo( temp, target, T, 5::ms );
    }

    fun static void gainTo( TapsTemp temp[], float target, dur T )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ gainTo( temp[i], target, T );
		T => now;
    }

    fun static void gainTo( TapsTemp temp, float target )
    {
        gainTo( temp, target, 0::second, 5::ms );
    }

    fun static void gainTo( TapsTemp temp[], float target )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ gainTo( temp[i], target );
    }

    fun static void spork_gainTo( TapsTemp temp, float target, dur T, dur inc )
    {
        <<< "sporking gainTo..." >>>;
        spork ~ gainTo( temp, target, T, inc );
    }

    fun static void spork_gainTo( TapsTemp temp, float target, dur T )
    {
        <<< "sporking gainTo..." >>>;
        spork ~ gainTo( temp, target, T );
    }


    fun static void freqTo( TapsTemp temp, float target, dur T, dur inc )
    {
        if( T > 0::second )
        {
            temp.freqWarp() => float curr;
            T / inc => float steps;
            target - curr => float diff;
            diff / steps => float val;
            float iter;

            <<< "setting freqWarp to:", target, "in", T/ms, "ms, inc:", inc/ms, "ms" >>>;
            while( iter < steps )
            {
                curr + iter * val => temp.freqWarp;
                1 +=> iter;
                inc => now;
            }
        }

        target => temp.freqWarp;
        <<< "freqWarp set to:", target >>>;
    }

    fun static void freqTo( TapsTemp temp[], float target, dur T, dur inc )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ freqTo( temp[i], target, T, inc );
		T => now;
    }
    
	fun static void freqTo( TapsTemp temp, float target, dur T )
    {
        freqTo( temp, target, T, 5::ms );
    }

    fun static void freqTo( TapsTemp temp[], float target, dur T )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ freqTo( temp[i], target, T );
		T => now;
    }

    fun static void freqTo( TapsTemp temp, float target )
    {
        freqTo( temp, target, 0::second, 5::ms );
    }

    fun static void freqTo( TapsTemp temp[], float target )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ freqTo( temp[i], target );
    }

    fun static void spork_freqTo( TapsTemp temp, float target, dur T, dur inc )
    {
        <<< "sporking freqTo..." >>>;
        spork ~ freqTo( temp, target, T, inc );
    }

    fun static void spork_freqTo( TapsTemp temp, float target, dur T )
    {
        <<< "sporking freqTo..." >>>;
        spork ~ freqTo( temp, target, T );
    }


    fun static void stretchTo( TapsTemp temp, float target, dur T, dur inc )
    {
        if( T > 0::second )
        {
            temp.timeStretch() => float curr;
            T / inc => float steps;
            target - curr => float diff;
            diff / steps => float val;
            float iter;

            <<< "setting timeStretch to:", target, "in", T/ms, "ms, inc:", inc/ms, "ms" >>>;
            while( iter < steps )
            {
                curr + iter * val => temp.timeStretch;
                1 +=> iter;
                inc => now;
            }
        }

        target => temp.timeStretch;
        <<< "timeStretch set to:", target >>>;
    }

    fun static void stretchTo( TapsTemp temp[], float target, dur T, dur inc )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ stretchTo( temp[i], target, T, inc );
		T => now;
    }

    fun static void stretchTo( TapsTemp temp, float target, dur T )
    {
        stretchTo( temp, target, T, 5::ms );
    }

    fun static void stretchTo( TapsTemp temp[], float target, dur T )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ stretchTo( temp[i], target, T );
		T => now;
    }

    fun static void stretchTo( TapsTemp temp, float target )
    {
        stretchTo( temp, target, 0::second, 5::ms );
    }

    fun static void stretchTo( TapsTemp temp[], float target )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ stretchTo( temp[i], target );
    }
    
	fun static void spork_stretchTo( TapsTemp temp, float target, dur T, dur inc )
    {
        <<< "sporking stretchTo..." >>>;
        spork ~ stretchTo( temp, target, T, inc );
    }

    fun static void spork_stretchTo( TapsTemp temp, float target, dur T )
    {
        <<< "sporking stretchTo..." >>>;
        spork ~ stretchTo( temp, target, T );
    }

    fun static void periodicityTo( TapsTemp temp, float target, dur T, dur inc )
    {
        if( T > 0::second )
        {
            temp.periodicity() => float curr;
            T / inc => float steps;
            target - curr => float diff;
            diff / steps => float val;
            float iter;

            <<< "setting periodicity to:", target, "in", T/ms, "ms, inc:", inc/ms, "ms" >>>;
            while( iter < steps )
            {
                curr + iter * val => temp.periodicity;
                1 +=> iter;
                inc => now;
            }
        }

        target => temp.periodicity;
        <<< "periodicity set to:", target >>>;
    }

    fun static void periodicityTo( TapsTemp temp[], float target, dur T, dur inc )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ periodicityTo( temp[i], target, T, inc );
		T => now;
    }

    fun static void periodicityTo( TapsTemp temp, float target, dur T )
    {
        periodicityTo( temp, target, T, 5::ms );
    }

    fun static void periodicityTo( TapsTemp temp[], float target, dur T )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ periodicityTo( temp[i], target, T );
		T => now;
    }

    fun static void periodicityTo( TapsTemp temp, float target )
    {
        periodicityTo( temp, target, 0::second, 5::ms );
    }

    fun static void periodicityTo( TapsTemp temp[], float target )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ periodicityTo( temp[i], target );
    }

    fun static void spork_periodicityTo( TapsTemp temp, float target, dur T, dur inc )
    {
        <<< "sporking periodicityTo..." >>>;
        spork ~ periodicityTo( temp, target, T, inc );
    }

    fun static void spork_periodicityTo( TapsTemp temp, float target, dur T )
    {
        <<< "sporking periodicityTo..." >>>;
        spork ~ periodicityTo( temp, target, T );
    }


    fun static void densityTo( TapsTemp temp, float target, dur T, dur inc )
    {
        if( T > 0::second )
        {
            temp.density() => float curr;
            T / inc => float steps;
            target - curr => float diff;
            diff / steps => float val;
            float iter;

            <<< "setting density to:", target, "in", T/ms, "ms, inc:", inc/ms, "ms" >>>;
            while( iter < steps )
            {
                curr + iter * val => temp.density;
                1 +=> iter;
                inc => now;
            }
        }

        target => temp.density;
        <<< "density set to:", target >>>;
    }

    fun static void densityTo( TapsTemp temp[], float target, dur T, dur inc )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ densityTo( temp[i], target, T, inc );
		T => now;
    }

    fun static void densityTo( TapsTemp temp, float target, dur T )
    {
        densityTo( temp, target, T, 5::ms );
    }

    fun static void densityTo( TapsTemp temp[], float target, dur T )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ densityTo( temp[i], target, T );
		T => now;
    }

    fun static void densityTo( TapsTemp temp, float target )
    {
        densityTo( temp, target, 0::second, 5::ms );
    }

    fun static void densityTo( TapsTemp temp[], float target )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ densityTo( temp[i], target );
    }

    fun static void spork_densityTo( TapsTemp temp, float target, dur T, dur inc )
    {
        <<< "sporking densityTo..." >>>;
        spork ~ densityTo( temp, target, T, inc );
    }

    fun static void spork_densityTo( TapsTemp temp, float target, dur T )
    {
        <<< "sporking densityTo..." >>>;
        spork ~ densityTo( temp, target, T );
    }


    fun static void randomTo( TapsTemp temp, float target, dur T, dur inc )
    {
        if( T > 0::second )
        {
            temp.random() => float curr;
            T / inc => float steps;
            target - curr => float diff;
            diff / steps => float val;
            float iter;

            <<< "setting random to:", target, "in", T/ms, "ms, inc:", inc/ms, "ms" >>>;
            while( iter < steps )
            {
                curr + iter * val => temp.random;
                1 +=> iter;
                inc => now;
            }
        }

        target => temp.random;
        <<< "random set to:", target >>>;
    }

    fun static void randomTo( TapsTemp temp[], float target, dur T, dur inc )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ randomTo( temp[i], target, T, inc );
		T => now;
    }

    fun static void randomTo( TapsTemp temp, float target, dur T )
    {
        randomTo( temp, target, T, 5::ms );
    }

    fun static void randomTo( TapsTemp temp[], float target, dur T )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ randomTo( temp[i], target, T );
		T => now;
    }

    fun static void randomTo( TapsTemp temp, float target )
    {
        randomTo( temp, target, 0::second, 5::ms );
    }

    fun static void randomTo( TapsTemp temp[], float target )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ randomTo( temp[i], target );
    }

    fun static void spork_randomTo( TapsTemp temp, float target, dur T, dur inc )
    {
        <<< "sporking randomTo..." >>>;
        spork ~ randomTo( temp, target, T, inc );
    }

    fun static void spork_randomTo( TapsTemp temp, float target, dur T )
    {
        <<< "sporking randomTo..." >>>;
        spork ~ randomTo( temp, target, T );
    }


    fun static void panTo( TapsTemp temp, float target, dur T, dur inc )
    {
        if( T > 0::second )
        {
            temp.pan() => float curr;
            T / inc => float steps;
            target - curr => float diff;
            diff / steps => float val;
            float iter;

            <<< "setting pan to:", target, "in", T/ms, "ms, inc:", inc/ms, "ms" >>>;
            while( iter < steps )
            {
                curr + iter * val => temp.pan;
                1 +=> iter;
                inc => now;
            }
        }

        target => temp.pan;
        <<< "pan set to:", target >>>;
    }

    fun static void panTo( TapsTemp temp[], float target, dur T, dur inc )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ panTo( temp[i], target, T, inc );
		T => now;
    }

    fun static void panTo( TapsTemp temp, float target, dur T )
    {
        panTo( temp, target, T, 5::ms );
    }

    fun static void panTo( TapsTemp temp[], float target, dur T )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ panTo( temp[i], target, T );
		T => now;
    }

    fun static void panTo( TapsTemp temp, float target )
    {
        panTo( temp, target, 0::second, 5::ms );
    }

    fun static void panTo( TapsTemp temp[], float target )
    {
	    for( int i; i < temp.cap(); i++ )
            spork ~ panTo( temp[i], target );
    }

    fun static void spork_panTo( TapsTemp temp, float target, dur T, dur inc )
    {
        <<< "sporking panTo..." >>>;
        spork ~ panTo( temp, target, T, inc );
    }

    fun static void spork_panTo( TapsTemp temp, float target, dur T )
    {
        <<< "sporking panTo..." >>>;
        spork ~ panTo( temp, target, T );
    }


    fun static void busReverbTo( int which, float target, dur T, dur inc )
    {
        if( T > 0::second )
        {
            TapsBus.reverb(which) => float curr;
            T / inc => float steps;
            target - curr => float diff;
            diff / steps => float val;
            float iter;

            <<< "setting reverb on bus", which, "to:", target, "in", T/ms, "ms, inc:", inc/ms, "ms" >>>;
            while( iter < steps )
            {
                TapsBus.reverb( which, curr + iter * val );
                1 +=> iter;
                inc => now;
            }
        }

        TapsBus.reverb( which, target );
        <<< "reverb on bus", which, "set to:", target >>>;
    }

    fun static void busReverbTo( int which, float target, dur T )
    {
        busReverbTo( which, target, T, 5::ms );
    }

    fun static void busReverbTo( int which, float target )
    {
        busReverbTo( which, target, 0::second, 5::ms );
    }

    fun static void timer( time start )
    {
        // infinite time loop
        while( true )
        {
            // print now in seconds
            <<< "tick:", ((now-start) / second), "seconds" >>>;
            1::second => now;
        }    
    }

    fun static void play( TapsTemp temp[], int bus )
	{
	    for( int i; i < temp.cap(); i++ )
		    temp[i].play( bus );
	}

    fun static void play( TapsTemp temp[] )
	{
	    for( int i; i < temp.cap(); i++ )
		    temp[i].play();
	}
	
	fun static void stop( TapsTemp temp[] )
	{
	    for( int i; i < temp.cap(); i++ )
		    temp[i].stop();
	}

    fun static void wait( TapsTemp temp )
    {
        while( temp.playing() )
        {
            50::ms => now;
        }
    }
}
