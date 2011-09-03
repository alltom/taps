//-----------------------------------------------------------------------------
// name: load.ck
// desc: loading things into synthesis face
//-----------------------------------------------------------------------------

// how many
4 => int N;
// name
"chirp1" => string name;

// count how many are in the library
TapsSynth.count( name ) => int num;
<<< "there are", num, "templates", name, "in the library" >>>;

// load if there is not enough
if( num < N )
{
    if( !TapsSynth.load( name ) )
    {
        <<< "load.ck: cannot load", name >>>;
        me.exit();
    }
    TapsSynth.copy( name, 0, N - num - 1 );
}
