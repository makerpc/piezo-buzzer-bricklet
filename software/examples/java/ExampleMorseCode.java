import com.tinkerforge.BrickletPiezoBuzzer;
import com.tinkerforge.IPConnection;

public class ExampleMorseCode {
	private static final String host = new String("localhost");
	private static final int port = 4223;
	private static final String UID = new String("ABC"); // Change to your UID
	
	// Note: To make the example code cleaner we do not handle exceptions. Exceptions you
	//       might normally want to catch are described in the commnents below
	public static void main(String args[]) throws Exception {
		// Create connection to brickd
		IPConnection ipcon = new IPConnection(host, port); // Can throw IOException

		BrickletPiezoBuzzer pb = new BrickletPiezoBuzzer(UID); // Create device object

		// Add device to ip connection
		ipcon.addDevice(pb); // Can throw IPConnection.TimeoutException
		// Don't use device before it is added to a connection
		

		// Morse SOS
		pb.morseCode("... --- ...");
		
		System.out.println("Press ctrl+c to exit");
		ipcon.joinThread();
	}
}