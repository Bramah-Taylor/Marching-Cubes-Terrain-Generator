# Marching Cubes Terrain Generator

[![Video demonstration](https://i.ytimg.com/vi/ZSce-SZAOx8/hqdefault.jpg)](https://youtu.be/ZSce-SZAOx8)

This repository contains code for a fully parallelised implementation of the marching cubes algorithm for usage in terrain generation, written in HLSL and using DirectX 11. All of the CPU-side DirectX code has been included in addition to the shader code.

The marching cubes implementation can be found in shaders/marching_cubes_gs.hlsl.

Implementations of the various noise algorithms can be found in the noise_fx.hlsl and gradient_noise_cs.hlsl shaders. The gradient noise shader also contains all of the volumetric data generation code.

## Requirements

This application was built using a lecturer-provided framework that is not publically available. However, all of the CPU-side compute shader code has been provided as that was not part of the initial framework, and the code provided should hopefully be easy enough to follow through without direct access to the framework's class implementations.

## Further Reading

- [Paul Bourke's ever-useful marching cubes reference and triangulation table](http://paulbourke.net/geometry/polygonise/)
- [Ryan Geiss's chapter in GPU Gems 3](https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch01.html) walks through parallelizing and optimising marching cubes for the GPU (though it's a bit out of date at this point)
- [Ryan Geiss's source code from his website](http://www.geisswerks.com/about_terrain.html)
- [Sebastian Lague's video on marching cubes](https://www.youtube.com/watch?v=M3iI2l0ltbE)
