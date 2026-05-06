# OpenGL Rendering Pipeline Study

## Introduction

This repository documents my practical study of the OpenGL rendering pipeline, focused on building a solid foundation in real-time graphics before moving on to lower-level APIs like DirectX 12. My goal was never to build a complete engine, but to understand the concepts behind each rendering technique by implementing them from scratch: how light interacts with surfaces, how shadows are projected, how normal maps simulate geometry, and how the GPU pipeline handles each step of that process.

Each implementation was built on top of a small custom framework I developed alongside the study, including a GLTF model loader, a camera system, and shader abstractions.

## Branch Structure

I organized the repository so that each branch isolates a single feature or technique. Some techniques require specific scene setups, shader configurations, or incompatible rendering contexts that would be difficult or time-consuming to reconcile into a single codebase. This structure makes it easier to study each concept independently and keeps the code focused. Think of each branch as a self-contained experiment, not a step in a linear build.

## How to Use This Repository

I recommend finding the learning format that works best for you first, whether that is video tutorials, written guides, or a combination. [LearnOpenGL](https://learnopengl.com) is an excellent written reference and covers most of the topics in this repository. Use this repository if you are looking for a different perspective on a concept, or if you are running into implementation issues in an environment similar to mine (Windows, Visual Studio, GLTF model loading with a custom loader). The code here reflects real decisions made during study, including edge cases, comments, and occasional refactoring.

## Controls

All scenes are navigable like a standard game engine viewport. Hold the right mouse button and move the mouse to rotate the camera. Use WASD to move horizontally, Q to move down and E to move up. Note that some earlier branches use the left mouse button for camera rotation instead.

## Branch Map

| Branch | Description | ImGUI | Refined Camera with persistence |
|---|---|---|---|
| `Face-Culling-FPS-Counter` | Introduction to face culling and an FPS counter in the window title | | |
| `Stencil-Buffer-and-Outlining` | Uses the stencil buffer to render an outline effect around objects using a double-model technique | | |
| `Transparency-and-Blending` | Implements transparency and alpha blending for windows meshes rendered in a loop at different rotations and distances | | |
| `Framebuffer-and-postprocessing` | Custom framebuffer with post-processing effects including pixelization and blur via fragment shader | | |
| `Cubemap-and-Skybox` | Cubemap loading and skybox rendering | | |
| `Geometry-Shader` | Introduction to the geometry shader stage, used to animate a grass blade swaying in the wind via shader | | |
| `Instancing` | Uses GPU instancing to render thousands of asteroids in a single draw call | | |
| `Blinn-Phong-Lighting` | Replaces the Phong specular model with Blinn-Phong using the halfway vector | | |
| `Anti-Aliasing` | Enables MSAA via GLFW multisampling hint and `glEnable(GL_MULTISAMPLE)` | | |
| `Gamma-Correction` | Implements gamma correction via sRGB texture formats and a post-processing framebuffer pass | | |
| `Shadow-Map` | Directional shadow mapping with a custom depth framebuffer, basic shadow and PCF soft shadows | ✓ | ✓ |
| `ImGui-Object-Editing` | Integrates Dear ImGUI for in-scene object inspection and transform editing | ✓ | ✓ |
| `Point-Light-Shadow-Mapping` | Omnidirectional shadow mapping using a cubemap depth buffer for point lights | ✓ | ✓ |
| `Spot-Light-Shadow-Mapping` | Shadow mapping adapted for a spot light with cubemap shadows and quaternion-based light rotation | ✓ | ✓ |
| `Normal-map` | Full normal mapping implementation with TBN matrix computed on the CPU and applied in the vertex shader | ✓ | ✓ |

Branches marked with Refined Camera include delta time movement, increased speed on shift, ESC to reset position, and camera state persistence across sessions via JSON.

## Why I Stopped at Normal Mapping

The core concepts of the rendering pipeline are covered here. Techniques like Parallax Mapping, HDR, Bloom, and Deferred Shading will be revisited during my DirectX 12 study, where the focus shifts to explicit GPU control and memory management. OpenGL was the right starting point because it abstracts enough complexity to let you focus on the concepts without getting lost in API details. If you are starting out, I recommend the same approach before moving to Vulkan or DX12.
