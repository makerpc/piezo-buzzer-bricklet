#!/usr/bin/perl  

use Tinkerforge::IPConnection;
use Tinkerforge::BrickletPiezoBuzzer;

use constant HOST => 'localhost';
use constant PORT => 4223;
use constant UID => 'xyz'; # Change to your UID

my $ipcon = Tinkerforge::IPConnection->new(); # Create IP connection
my $pb = Tinkerforge::BrickletPiezoBuzzer->new(&UID, $ipcon); # Create device object

$ipcon->connect(&HOST, &PORT); # Connect to brickd
# Don't use device before ipcon is connected

# Make 2 second beep
$pb->beep(2000);

print "Press any key to exit...\n";
<STDIN>;
$ipcon->disconnect();

