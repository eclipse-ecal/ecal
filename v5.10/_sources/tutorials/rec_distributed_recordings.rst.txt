
====================================
Distributed Recordings with eCAL Rec
====================================

.. youtube:: h3DUirOMfu8

Transcript
==========

Welcome to this tutorial where I am going to show you how to create a distributed measurement with the eCAL Recorder and how to merge two measurements afterwards.
But before we start let's clarify really quick what is a distributed measurement and why should you create one.
So imagine you have two or more PCs and each PC is publishing a large amount of data and you want to record everything so you can now of course launch one recorder instance on one PC
and let it record everything but that will cause the data to flow over the network and especially if you have very large messages for example a video stream that may cause problems because your network may be overwhelmed, you may drop packages and miss whole frames and you will get an incomplete measurement afterwards.
So what you can do is create a distributed measurement where you launch one recorder per PC, let each PC record its own recording and afterwards merge all of those individual recordings and get one fused recording.
Okay? So that is what we're going to do today!
So let's start the eCAL Recorder on the Windows machine here and what you can already see is that it is publishing my system here is publishing two topics the hello topic from my Windows machine and the person topic from my Ubuntu machine and we can do a centralized recording now directly by hitting record.
But we want to do a distributed recording, so on Ubuntu let's start the eCAL Rec client that can also record and as you can see in the recorder panel on my Windows machine here it already picked up and i can now check it and now i have activated both recorders.
At the moment it will still create two entire recordings so if we fuse those um we will get everything twice and that's why it displays all of these warning signs.
So I have to click here and select for the Ubuntu machine that it shall create all of the Ubuntu topics or I can also click the local topics button here and it will do everything automatically.
If you ever end up with a configuration that records a topic multiple times and duplicates all of those messages the eCAL Recorder will also complain in this thing on the left in the topics panel so you know and can change your configuration.
What you also have to make sure is in a distributed recording to time synchronize all of your machines.
Because if you don't if you for example your machines are let's say five seconds off then all of your frames from your measurement if you fuse those measurements together will then be five seconds off and so the measurement will not be very useful for you, so make sure your systems are time synchronized and the eCAL Recorder will also complain if they are not.
I have made sure by just checking the little time synchronization checkbox in my virtual machine so everything is fine here and so we can now directly hit record!
So let's do that, let's start recording!
It will create this measurement item down here, I can expand it and see which recorder instance is recording how many frames and if i now hit stop after a few seconds we can already see the measurement here, it is in C:\ecal_meas\ on windows and it contains the directory from my windows machine and on Ubuntu there is in the home directory the ecal_meas and it contains a directory for the Ubuntu frames.
So we have to fuse those two together and all that takes is copying the files over.
It is a pure file based merging.
You can do that on your own by just using a USB stick or pulling them over a network share or something like that, you can do that, but the recorder also supports it, you just have to click this little upload button here and all it does is upload from the clients, so in this case only this one client here, to your main PC which by default is the graphical user interface PC and it will put it in the same directory that the original measurement was in.
So let's click upload and it already finished so if we now check the measurement directory we now have both directories, the Ubuntu and the Windows directory and so this is a merged measurement!
If we load it in the car player let's drop it in we can see the measurement contains all topics from channel hello and from channel person.
And so that was already everything!
This is how you create a distributed recording and so if this video helped you you can let me know in the comments or also let me know if something was missing and maybe see you in the next video!
