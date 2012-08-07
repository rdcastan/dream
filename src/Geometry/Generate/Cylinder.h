//
//  Geometry/Generate/Cylinder.h
//  This file is part of the "Dream" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 9/03/07.
//  Copyright (c) 2007 Samuel Williams. All rights reserved.
//
//

#ifndef _DREAM_GEOMETRY_GENERATE_CYLINDER_H
#define _DREAM_GEOMETRY_GENERATE_CYLINDER_H

#include "Basic.h"
#include "../Spline.h"

namespace Dream {
	namespace Geometry {
		namespace Generate {
			RealT calculate_rotation_length (const std::size_t rotation, const std::vector<Vec3> &left, const std::vector<Vec3> &right) {
				RealT length = 0;

				Vec3 centers[2];
				centers[0].zero(); centers[1].zero();

				for (std::size_t i = 0; i < left.size(); i += 1) {
					centers[0] += left[i];
					centers[1] += right[i];
				}

				centers[0] /= left.size();
				centers[1] /= right.size();

				Vec3 segment_direction = (centers[1] - centers[0]).normalize();

				for (std::size_t i = 0; i < left.size(); i += 1) {
					RealT cosl = segment_direction.dot((right[(i + rotation) % right.size()] - left[i]).normalize());
					length += 1.0 - cosl;
				}

				return length;
			}

			int calculate_rotation_offset (const std::vector<Vec3> &left, const std::vector<Vec3> &right) {
				DREAM_ASSERT(left.size() == right.size());
				std::vector<RealT> lengths(left.size());

				for (std::size_t i = 0; i < right.size(); i++) {
					lengths.at(i) = calculate_rotation_length(i, left, right);
				}

				int index = 0;
				RealT maximum = lengths[0];
				for (std::size_t i = 1; i < lengths.size(); ++i) {
					if (maximum > lengths[i]) {
						index = i;
						maximum = lengths[i];
					}
				}

				return index;
			}

			/// Build a tube out of a set of stacks of a given number of slices:
			template <typename MeshT>
			void tube(MeshT & mesh, std::size_t slices) {
				for (std::size_t i = 0; i < (mesh.vertices.size() - slices); i += slices) {
					for (std::size_t j = 0; j < slices; j += 1) {
						std::size_t k = (j + 1) % slices;

						std::size_t i1 = i + j, j1 = (i + slices) + j;
						std::size_t i2 = i + k, j2 = (i + slices) + k;

						mesh << i1 << i2 << j2;
						mesh << i1 << j2 << j1;
					}
				}
			}

			template <typename MeshT>
			void cylinder(MeshT & mesh, const RealT &base_radius, const RealT &top_radius, const RealT &height, const std::size_t &slices, const std::size_t &stacks, bool cap_start, bool cap_end) {
				mesh.layout = TRIANGLES;

				Vec3 base (0, base_radius, 0);
				Vec3 top (height, top_radius, 0);
				Vec3 center(1, 0, 0);

				DREAM_ASSERT((base_radius != 0.0 || top_radius != 0.0) && "Both base_radius and top_radius are zero!");

				auto rotation = Mat44::rotating_matrix(-R360 / slices, center);

				// N stacks has n+1 divisions, one for the start, one for the end
				std::size_t divisions = stacks + 1;

				//auto index = array_index<2>(vec(divisions, slices));
				//ArrayIndex<2> index(vec(divisions, slices));
				//index(mesh->vertices)[y][x]

				mesh.vertices.reserve(divisions * slices);

				// Generate all the points for the stacks and slices of the cylinder
				for (std::size_t stk = 0; stk < divisions; ++stk) {
					Vec3 current = linear_interpolate((RealT)stk / (RealT)stacks, base, top);

					for (std::size_t slice = 0; slice < slices; ++slice) {
						typename MeshT::VertexT vertex;
						vertex.position = current;

						mesh << vertex;

						current = rotation * current;
					}
				}

				/*
				// If we are capping, add two more points for the center points at each end
				if (cap_start || cap_end) {
				    typename MeshT::VertexT vertex;
				    vertex.position.zero();

				    mesh.vertices.push_back(vertex);

				    vertex.position[X] = height;
				    mesh.vertices.push_back(vertex);

				    unsigned base = 0, top = (divisions - 1) * slices;
				    unsigned centers = mesh.vertices.size() - 2;

				    for (std::size_t slice = 0; slice < slices; ++slice) {
				        // Add in triangles for the caps
				        if (cap_start && base_radius > 0.0) {
				            mesh.indices.push_back(centers+0);
				            mesh.indices.push_back(base + ((slice+1) % slices));
				            mesh.indices.push_back(base + slice);
				        }

				        if (cap_end && top_radius > 0.0) {
				            mesh.indices.push_back(centers+1);
				            mesh.indices.push_back(top + slice);
				            mesh.indices.push_back(top + ((slice+1) % slices));
				        }
				    }
				}
				*/

				/*
				// Add in the quad surfaces for the cylinder
				for (std::size_t stk = 0; stk < stacks; ++stk) {
				    std::size_t stack_bottom_index = stk * slices;
				    std::size_t stack_top_index = (stk + 1) * slices;

				    for (std::size_t slice = 0; slice < slices; slice += 1) {
				        std::size_t slice_current_offset = slice;
				        std::size_t slice_next_offset = (slice + 1) % slices;

				        if (base_radius == 0.0 && stk == 0) {
				            mesh.indices.push_back(stack_top_index + slice_current_offset);
				            mesh.indices.push_back(stack_bottom_index + slice_current_offset);
				            mesh.indices.push_back(stack_top_index + slice_next_offset);
				        } else if (top_radius == 0.0 && stk == (stacks-1)) {
				            mesh.indices.push_back(stack_top_index + slice_current_offset);
				            mesh.indices.push_back(stack_bottom_index + slice_current_offset);
				            mesh.indices.push_back(stack_bottom_index + slice_next_offset);
				        } else {
				            mesh.indices.push_back(stack_top_index + slice_current_offset);
				            mesh.indices.push_back(stack_bottom_index + slice_current_offset);
				            mesh.indices.push_back(stack_bottom_index + slice_next_offset);

				            mesh.indices.push_back(stack_top_index + slice_current_offset);
				            mesh.indices.push_back(stack_bottom_index + slice_next_offset);
				            mesh.indices.push_back(stack_top_index + slice_next_offset);
				        }
				    }
				}
				*/

				tube(mesh, slices);
			}

			/// Construct a tube out of a series of splines:
			template <typename MeshT, typename SplineT>
			void cylinder(MeshT & mesh, const std::vector<SplineT> & splines, RealT resolution) {
				std::size_t slices = splines.size();
				resolution = 1.0 / resolution;

				for (RealT i = 0.0; i <= 1.0; i += resolution) {
					for (std::size_t c = 0; c < slices; c += 1) {
						typename MeshT::VertexT vertex;
						vertex.position = splines[c].point_at_time(i);

						mesh << vertex;
					}
				}

				tube(mesh, splines.size());
			}
		}
	}
}

#endif
