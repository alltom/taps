// name: bus.ck
// desc: tapestrea bus control test
//       run this in synthesis face, and switch to control to observe

// number of buses
	8 => int N;

// volume
for( int i; i < N; i++ )
	<<< "volume:", i, TapsBus.volume( i ) >>>;

// volume
for( int i; i < N; i++ )
	<<< "pan:", i, TapsBus.pan( i ) >>>;

// volume
for( int i; i < N; i++ )
	<<< "reverb:", i, TapsBus.reverb( i ) >>>;

.01 => float inc;
(1 / inc) $ int => int steps;
5::ms => dur T;

for( int i; i < N; i++ )
{
    repeat(steps)
	{
	    TapsBus.volume( i, TapsBus.volume(i)-inc );
		T => now;
	}
    repeat(steps)
	{
	    TapsBus.volume( i, TapsBus.volume(i)+inc );
		T => now;
	}
}

for( int i; i < N; i++ )
{
    repeat(steps)
	{
	    TapsBus.reverb( i, TapsBus.reverb(i)+inc );
		T => now;
	}
    repeat(steps)
	{
	    TapsBus.reverb( i, TapsBus.reverb(i)-inc );
		T => now;
	}

	if( TapsBus.reverb(i) < 0.0 ) TapsBus.reverb( i, 0.0 );
}

/*
for( int i; i < N; i++ )
{
    repeat(steps)
	{
	    TapsBus.pan( i, TapsBus.pan(i)-inc );
		T => now;
	}
    repeat(steps)
	{
	    TapsBus.pan( i, TapsBus.pan(i)+inc );
		T => now;
	}
}
*/

repeat(steps)
{
    for( int i; i < N; i++ )
    {
        TapsBus.volume( i, TapsBus.volume(i)-inc );
    }
    T => now;
}

repeat(steps)
{
    for( int i; i < N; i++ )
    {
        TapsBus.volume( i, TapsBus.volume(i)+inc );
    }
    T => now;
}

repeat(steps)
{
    for( int i; i < N; i++ )
    {
        TapsBus.reverb( i, TapsBus.reverb(i)+inc );
    }
    T => now;
}

repeat(steps)
{
    for( int i; i < N; i++ )
    {
        TapsBus.reverb( i, TapsBus.reverb(i)-inc );
    }
    T => now;
}


