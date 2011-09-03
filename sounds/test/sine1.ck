sinosc s => dac;
.25 => s.gain;

0.0 => float t;

while( true )
{
    (100.0 + std.fabs(300.0 * math.sin(t))) => s.freq;
    t + .01 => t;
    5::ms => now;
}
