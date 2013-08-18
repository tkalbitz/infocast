# Infocast

## About

Infocast is a self contained daemon used by the Nao-Team HTWK to see which 
robots are connected to the network and to get some basic information. 
This includes:

* Hostname
* Ethernet MAC and IP
* Wifi MAC and IP
* CPU temperature
* Battery status
* Wifi signal strength

It also includes a UDP port which can be used for the text to speech capabilities of the bot.

## Building

Just add the infocast subdirectory to your building environment via the CMake
statement:

<pre>add_subdirectory( "infocast" )</pre>

You have to build it with the cross compile environment because of the 
Aldebaran library dependencies.

Everything to run infocast as a daemon can be found in subdirectory extras.

## Packet Format 

<pre>
 -------------------------------------------------------------------------------
| magic byte | version  | lan ip | wifi ip | battery | cpu temp | wifi strength |
|   uint8    |  uint8   | uint32 | uint32  |  int8   |  uint8   |    uint8      |
 -------------------------------------------------------------------------------
|   lan mac  | wifi mac |         host name          |  
|  6x uint8  | 6x uint8 |    len(host name) + '\0'   |
 ----------------------------------------------------
</pre>

This format might change in the future to protobuf. All multi byte fields are in
network byteorder. The battery field is signed to indicate that the robot is 
charging (positive value) or discharging (negative value). 

## Example GUI
There exists an example QT4 GUI which is called NaoFinder. You can find it also
on GitHub: https://github.com/tkalbitz/NaoFinder
