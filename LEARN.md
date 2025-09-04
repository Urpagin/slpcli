# Simple Steps To Attempt The Same

And how I think this project is awesome as a first in networking.

## A Bit of Binary

Roughly know how things are stored on a computer, in the RAM, on disk, data types.

How a normal unsigned integer is represented using bits.


## Sockets & Packets

First, I think you should very quickly get a grasp on sockets and TCP packets, for this is the basis on which
we'll build our project.


Read a few summaries on how it works; I prefer doing something over too much theory. When you think you know how sockets
and packets works, make a quick two-components Python script just to fiddle around with sockets. Make a quick client and server,
and no need for asynchronous.


Understand a socket is just like a file, but that which cannot be sought; the data streams to or from the socket.

You either send it or receive it.

## Protocol Packets & Protocol Data Types

Once you've done that, now you start reading the [Minecraft: Java Edition protocol wiki](https://minecraft.wiki/w/Java_Edition_protocol/).

For this project you'll only need to know these atoms:

* [Encoding and decoding of VarInts](https://minecraft.wiki/w/Java_Edition_protocol/Data_types#Type:VarInt) 
 (this could be optional if you're acknowledging that you cannot represent strings larger than 128 characters (7-bits)).

* [The order in which packets are received and sent](https://minecraft.wiki/w/Java_Edition_protocol/FAQ#What_does_the_normal_status_ping_sequence_look_like?), with us being **the client**.

* The individual packets to form and send, and the ones to represent and parse when reading from the socket. 
Click on the link above, then click on the individual packets to see how they're represented.


In theory, querying a Notchian (Java Edition) server could only require **one** sent packet (the Handshake packet),
and **one** received packet (the Status Response packet). So, try to do this first. Or just implement the normal way
of querying a server by implementing a few other packets. And do not fret! The most difficult one is the first, the Handshake packet,
mainly because it requires (strongly advised) implementing VarInts.


## Implementation

Select a programming language of your choosing, Python (very good start), Go, Ruby, Rust, C++, C, Cobol, Assembly, ... you name it.

I personally chose C++ just because it's freaking cool! And as I say in the `README.md`, you can shoot your leg off and you won't even
notice it! And that's the beauty of it.


## More Information - Walkthrough

This project, slpcli, is taking the stead of a Notchian **client**. But, you may want to see what's on the opposite end, being the **server**:

I wrote this blog a while back that explains the motives and how to implement a **server-side** SLP protocol. If you're curious, here's the link:

https://blog.urpagin.net/coding-a-minecraft-honeypot/


## Afterwords

If you do decide to attempt this project, which is challenging or trivial depending your current understanding of things.

I regardless, wish you happy coding!

Best of luck!!
Urpagin.
