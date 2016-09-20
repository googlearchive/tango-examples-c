Project Tango C API Example Projects
===========================================
Copyright 2014 Google Inc.

Useful Websites
---------------
SDK Download - https://developers.google.com/project-tango/downloads

Developer Website - https://developers.google.com/project-tango/apis/c

Contents
--------

This contains the Project Tango C API examples.

These examples use the Gradle build system and were developed using
Android Studio 2.1.

#### Basic Examples

The **cpp_basic_examples** project includes basic examples showing how
to compile and run an application using C/C++:

 * **hello_area_description** - Use the Area Description
   API to create and manage Area Description Files.
 * **hello_depth_perception** - Use the depth sensor.
 * **hello_motion_tracking** - Use the Motion Tracking API
   to track the position of the Tango device in 3D space.
 * **hello_video** - Render the RGB camera image using OpenGL.

#### Use Case Examples

Other examples in this repository show how to build an application for
different use cases of Tango technology:

 * **cpp_augmented_reality_example** - Achieve an augmented
   reality effect by rendering 3D objects overlaid on the camera image
   such that they appear to stay affixed in space.
 * **cpp_mesh_builder_example** - Use the depth sensor to
   build a mesh of the surrounding space.
 * **cpp_motion_tracking_example** - Use Tango motion
   tracking to navigate in a virtual 3D world.
 * **cpp_plane_fitting_example** - Build an AR application
   to detect planes in the real world to place objects in them.
 * **cpp_point_cloud_example** - Acquire and render a cloud
   of 3D points using the depth sensor.
 * **cpp_point_to_point_example** - Build a simple point-to-point
   measurement application using augmented reality and the depth
   sensor.
 * **cpp_rgb_depth_sync_example** - Synchronize the depth
   sensor 3D information with the color camera information.
 * **cpp_video_stabilization_experiment** - Stabilize the video by
   smoothing the pose and correcting for gravity.

The **cpp_example_util** project contains some common utility code that
is used for many samples.

Support
-------
As a first step, view our [FAQ](http://stackoverflow.com/questions/tagged/google-project-tango?sort=faq&amp;pagesize=50)
page. You can find solutions to most issues there.

If you have general API questions related to Tango, we encourage you to
post your question to our [stack overflow
page](http://stackoverflow.com/questions/tagged/google-project-tango).

To learn more about general concepts and other information about the
project, visit [Project Tango Developer website](https://developers.google.com/project-tango/).

Contribution
------------
Want to contribute? Great! First, read this page (including the small
print at the end).

#### Before you contribute
Before we can use your code, you must sign the
[Google Individual Contributor License
Agreement](https://developers.google.com/open-source/cla/individual?csw=1)
(CLA), which you can do online. The CLA is necessary mainly because you
own the
copyright to your changes, even after your contribution becomes part of
our
codebase, so we need your permission to use and distribute your code. We
also
need to be sure of various other things—for instance, that you'll tell us
if you
know that your code infringes on other people's patents. You don't have
to sign
the CLA until after you've submitted your code for review and a member
has
approved it, but you must do it before we can put your code into our
codebase.
Before you start working on a larger contribution, you should get in
touch with
us first through the issue tracker with your idea so that we can help
out and
possibly guide you. Coordinating up front makes it much easier to avoid
frustration later on.

#### Code reviews
All submissions, including submissions by project members, require
review. We
use Github pull requests for this purpose.

#### The small print
Contributions made by corporations are covered by a different agreement
than
the one above: the Software Grant and Corporate Contributor License
Agreement.
