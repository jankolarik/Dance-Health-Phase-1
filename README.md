# ReadMe Deployment Manual and How-to

# for Systems Engineering: Dance Health

## Team 32: Alex Tcherdakoff, Jan Kolarik, and Yide Fan

## Contents

1 Summary 1

2 Manual and How-To 1
2.1 Deployment.................................................. 1
2.1.1 Apple Watch Application...................................... 2
2.1.2 Kinect 2 Application......................................... 2
2.2 General Use.................................................. 2

3 Licensing 3
3.1 Apple Watch Examples........................................... 3
3.1.1 Speedy Sloth Example........................................ 3
3.2 Kinect Libraries and Software........................................ 4
3.2.1 Curl.................................................. 4
3.2.2 OpenGL................................................ 4
3.2.2.1 NupenGL.......................................... 4
3.2.2.2 FreeGlut.......................................... 5
3.2.2.3 GLFW........................................... 5
3.2.2.4 GLEW........................................... 5
3.2.2.4.1 GLEW...................................... 5
3.2.2.4.2 MESA...................................... 6
3.2.2.4.3 Khronos..................................... 6
3.2.3 OpenCV................................................ 7
3.2.4 Dear ImGui.............................................. 7
3.2.5 Kinect SDK and Library....................................... 8
3.2.6 FFMpeg Software and Library................................... 8

4 Acknowledgements 8

## 1 Summary

Dance Health is a student-led project that aims to create software for the Kinect 2 Camera and the Apple Watch
using the depth camera and skeletal tracking as well as Apple HealthKit data to evaluate health metrics and skill.

We are now approaching the end of the development stage of the project. To track our progress, plans, and
ideas, you can check us out [here](http://students.cs.ucl.ac.uk/2019/group32/index.html).

## 2 Manual and How-To

## 2.1 Deployment

General Prerequesites: You will need both the code for the Apple Watch, and the .exe file (or the Visual Studio
solution) for the Kinect. Find these in our github repo ([you're already here](https://github.com/jankolarik/Dance-Health-Phase-1)).


2.1.1 Apple Watch Application

Prerequisites: A Mac computer running the latest possible MacOS with Xcode installed, an iPhone with an Apple
ID Account and an Apple Watch. All devices used must be updated to the latest software.

Instructions:
Open the project in Xcode.

If you don’t have an Apple Watch, you can run it in the simulator (without having to do any of the following
signings): just open the project in Xcode, then at the top left corner of Xcode instead of Generic watchOS device,
select a specific simulator WatchOS device. Select your device and click the run button on the top left; the code
should install in approximately one minute.Dance Health Tracking app will be ready on the Apple Watch simulator.

If you do have an Apple Watch:
On the top left of Xcode, click on the navigator called DanceHealthTracking with the blue blueprint icon on its left.

In the signing and capabilities tab in the top middle part of you screen, click on the automatically manage signing
and select you Team to your Apple ID. In case it is not there automatically, click on add account and add your
account.
Connect your iPhone device to your mac via a charging cable and leave the iPhone unlocked. On the iPhone it
might ask you if you trust this computer. Please press yes. On the top left corner of Xcode instead of Generic
watchOS device, you should be able to select you Apple Watch device. Select your device and click the run button
on the top left; the code should install in approximately one minute. Once this is completed, feel free to disconnect
any devices and the Dance Health Tracking app will be ready on your Apple Watch.
At first initialization, it will ask you to grant it certain permissions.
After accepting these permissions, press the start button and start dancing!

2.1.2 Kinect 2 Application

Prerequisites: A Kinect 2 (for Windows) with all the appropriate cables to connect to your computer, and a com-
puter with the latest version of Windows installed, and/or the appropriate drivers for the Kinect 2. Maybe: the
latest version of Microsoft Visual Studio.

Instructions:
Plug the Kinect into your computer.
Open the .exe file to start your program.
Alternatively, open the solution (.sln) file with Visual Studio and run (under Release x64).
On the pop up window, follow the instruction and enter an appropriate session ID.
To end the Analysis and Video, stand out of the frame for 7 seconds and the program will stop automatically.
The Video recorded will be stored locally in the solution folder

### 2.2 General Use

Once you’ve deployed/installed the software, you might want to use it!

Start by opening the application on both of your devices.
Both will prompt for a session ID

The session ID is recommended to follow the convention: start with your first name e.g. ”Mike”, followed by
the number of the seesion you have done today e.g. if it’s this is the second session you have done today, enter ”2”,
so the result session ID should be ”mike2”.

The session ID is used for merging the data sent from Kinect and Apple Watch, so please make sure you en-
ter the same ID on both device for each session (this is very important).


This convention is used to make sure each session ID is unique on that day, you can also define your own convention.

Once you’ve input the session ID, the Kinect will open a start window with instructions on how to end the session,
and a required telemetry and data collection consent message.
The Apple Watch will display the metrics that will be measured, and ”Start Dancing” button.
Start your session by clicking both the ”start” button on the Kinect interface, and the ”Start Dancing” button on
the Apple Watch interface.

You can end the session on the Kinect by leaving frame for 7 seconds or more, or pressing the ”End Session”
button, which you can hide from the resulting video by closing the popup that contains this button.
For the Apple Watch, just click ”Stop Dancing”.

You can then view, delete, or manually enter your data on [our server](http://51.11.52.98:3300/).
The video from the Kinect should also be saved locally on your machine to whichever folder the Kinect application
files are placed.
Enjoy!

## 3 Licensing

This project is part of the ”Dance Health Application Package” developed by Team32 (Jan Kolarik, Yide Fan and
Alex Tcherdakoff) and is under the MIT license (MIT)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated docu-
mentation files (the ”Software”), to deal in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of
the Software.
THE SOFTWARE IS PROVIDED ”AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PAR-
TICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
A number of open source works were ussed in the production of this work; their licenses are as follows.

### 3.1 Apple Watch Examples

3.1.1 Speedy Sloth Example

The license for the Speedy Sloth code that is labelled was modified and partially used for this project is below.
Copyright 2019 Apple Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated docu-
mentation files (the ”Software”), to deal in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of
the Software.

THE SOFTWARE IS PROVIDED ”AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PAR-
TICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT


#### HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION

#### OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE

#### SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

### 3.2 Kinect Libraries and Software

3.2.1 Curl

COPYRIGHT AND PERMISSION NOTICE

Copyright (c) 1996 - 2020, Daniel Stenberg, daniel@haxx.se, and many contributors, see the THANKS file. [https://curl.haxx.se/docs/thanks.html]

All rights reserved.

Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted,
provided that the above copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED ”AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PAR-
TICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall not be used in advertising or other-
wise to promote the sale, use or other dealings in this Software without prior written authorization of the copyright
holder.

3.2.2 OpenGL

3.2.2.1 NupenGL

Copyright (c) 2014, NupenGL
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that
the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer. * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials
provided with the distribution.
Neither the name of NupenGL Project nor the names of its contributors may be used to endorse or promote prod-
ucts derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ”AS IS” AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WAR-
RANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBIL-
ITY OF SUCH DAMAGE.


3.2.2.2 FreeGlut

Freeglut code without an explicit copyright is covered by the following copyright:

Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved. Permission is hereby granted, free of charge,
to any person obtaining a copy of this software and associated documentation files (the ”Software”), to deal in the
Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies or substantial portions of the Software.
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.
THE SOFTWARE IS PROVIDED ”AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PAR-
TICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PAWEL W. OLSZTA BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.
Except as contained in this notice, the name of Pawel W. Olszta shall not be used in advertising or otherwise to
promote the sale, use or other dealings in this Software without prior written authorization from Pawel W. Olszta.

#### 3.2.2.3 GLFW

Copyright (c) 2002-2006 Marcus Geelnard
Copyright (c) 2006-2010 Camilla Berglund
<elmindreda@elmindreda.org>

This software is provided ’as-is’, without any express or implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and
to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment in the product documentation would be appreciated but
is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the orig-
inal software.
3. This notice may not be removed or altered from any source distribution.

#### 3.2.2.4 GLEW

The 3 sub-libraries of Glew used are as follows:

3.2.2.4.1 GLEW The OpenGL Extension Wrangler Library
Copyright (C) 2002-2008, Milan Ikits<milan ikits@ieee org>
Copyright (C) 2002-2008, Marcelo E. Magallon
<mmagallo@debian org>
Copyright (C) 2002, Lev Povalahev
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that
the following conditions are met:

Redistributions of source code must retain the above copyright notice,


this list of conditions and the following disclaimer. * Redistributions in binary form must reproduce the above
copyright notice,
this list of conditions and the following disclaimer in the documentation and/or other materials provided with the
distribution.
The name of the author may be used to endorse or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ”AS IS” AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WAR-
RANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBIL-
ITY OF SUCH DAMAGE.

3.2.2.4.2 MESA Mesa 3-D graphics library
Version: 7.

Copyright (C) 1999-2007 Brian Paul All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated docu-
mentation files (the ”Software”), to deal in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of
the Software.

THE SOFTWARE IS PROVIDED ”AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PAR-
TICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL BRIAN PAUL BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

3.2.2.4.3 Khronos Copyright (c) 2007 The Khronos Group Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and/or associated
documentation files (the ”Materials”), to deal in the Materials without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Materials, and to permit
persons to whom the Materials are furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of
the Materials.

THE MATERIALS ARE PROVIDED ”AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IM-
PLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR
A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPY-
RIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
THE MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.


3.2.3 OpenCV

By downloading, copying, installing or using the software you agree to this license.
If you do not agree to this license, do not download, install, copy or use the software.

License Agreement
For Open Source Computer Vision Library
(3-clause BSD License)

Copyright (C) 2000-2019, Intel Corporation, all rights reserved.
Copyright (C) 2009-2011, Willow Garage Inc., all rights reserved.
Copyright (C) 2009-2016, NVIDIA Corporation, all rights reserved.
Copyright (C) 2010-2013, Advanced Micro Devices, Inc., all rights reserved.
Copyright (C) 2015-2016, OpenCV Foundation, all rights reserved.
Copyright (C) 2015-2016, Itseez Inc., all rights reserved.
Copyright (C) 2019, Xperience AI, all rights reserved.
Third party copyrights are property of their respective owners.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that
the following conditions are met:
Redistributions of source code must retain the above copyright notice, this list of conditions and the following
disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
disclaimer in the documentation and/or other materials provided with the distribution.

Neither the names of the copyright holders nor the names of the contributors may be used to endorse or promote
products derived from this software without specific prior written permission.

This software is provided by the copyright holders and contributors ”as is” and any express or implied warranties,
including, but not limited to, the implied warranties of merchantability and fitness for a particular purpose are
disclaimed. In no event shall copyright holders or contributors be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss
of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if
advised of the possibility of such damage.

3.2.4 Dear ImGui

The MIT License (MIT)

Copyright (c) 2014-2020 Omar Cornut

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated docu-
mentation files (the ”Software”), to deal in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of
the Software.

THE SOFTWARE IS PROVIDED ”AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PAR-


#### TICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT

#### HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION

#### OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE

#### SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

3.2.5 Kinect SDK and Library

Read the license at: https://download.microsoft.com/download/0/D/C/0DC5308E-36A7-4DCD-B299-B01CDFC8E345/Kinect-SDK2.0-EULA_en-US.pdf.

3.2.6 FFMpeg Software and Library

Copyright (C) 2001 Fabrice Bellard

FFmpeg is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option)
any later version.

FFmpeg is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with FFmpeg; if not, write
to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

See the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1, February 1999 at: [https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html](here).

## 4 Acknowledgements

We (as Team 32) would like to express our gratitude to our project partners Arthur Murray Dance Studios, and
specifically to Adrian and Shaun Persad, for keeping clear and open communication lines, giving constant feedback,
and making the creation of this project so enjoyable and smooth.
We’d also like to thank our fellow Year 2 UCL Computer Science BSc/MEng Students for helping us user-test our
system at various stages and offering detailed feedback on different levels:
Rachel Mattoo, Dao Heng Liu, Rajesh Goyal, Joel Morgan, Swechha Kansakar, Sabina-Maria Mitroi, Mark Anson,
Govind Balla, Rikaz Rameez, Eesha Irfan, Daniel Lahlafi, Sara Maria, and Muna Aghamelu.


