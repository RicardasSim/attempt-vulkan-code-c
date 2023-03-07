#!/bin/bash

glslangValidator -V 27_shader_depth.vert -o shaders/vert.spv
glslangValidator -V 27_shader_depth.frag -o shaders/frag.spv
