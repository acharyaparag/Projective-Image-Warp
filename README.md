Projective-Image-Warp
=====================

Implemented a warping tool based on projective warps which included all the projective transformations using C++ and OpenImageIO. Separate procedures were written for constructing each of the projective transformations:translate,rotate,scale,flip,shear and perspective.The tool will provide for input of a sequence of operations,will compose a single transform from the sequence, and finally apply the transform to the input image. An Interactive warper was also created that allowed the user to interactively position the corners of the output warped quadrilateral and then compute the projective warp using those positions
