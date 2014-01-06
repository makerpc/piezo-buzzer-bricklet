Imports Tinkerforge

Module ExampleBeep
    Const HOST As String = "localhost"
    Const PORT As Integer = 4223
    Const UID As String = "6tU" ' Change to your UID

    Sub Main()
        Dim ipcon As New IPConnection() ' Create IP connection
        Dim pb As New BrickletPiezoBuzzer(UID, ipcon) ' Create device object

        ipcon.Connect(HOST, PORT) ' Connect to brickd
        ' Don't use device before ipcon is connected

        ' Make 2 second beep
        pb.Beep(2000)

        System.Console.WriteLine("Press key to exit")
        System.Console.ReadKey()
        ipcon.Disconnect()
    End Sub
End Module