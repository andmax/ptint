/**
 *   First Step Fragment Shader
 *
 *  Authors:
 *    Maximo, Andre
 *    Marroquim, Ricardo
 *
 *  Date: Jan-May, 2006
 *
 */

/**
 *   firstStep.frag :
 *     [1] Read the four vertices from Vertex Texture (5 access);
 *     [2] Compute tetrahedron baricenter;
 *     [3] Project tetrahedron vertices;
 *     [4] Classify the projectino (1 access);
 *     [5] Compute thick vertex;
 *     [6] Compute cell thickness, scalar front and back;
 *     [7] Write back the results in 2 FBO.
 *
 * GLSL code.
 *
 */

#extension GL_ARB_draw_buffers : enable ///< Enable multiple FBOs

uniform sampler2D tetrahedralTex; ///< Tetrahedra Texture
uniform sampler2D vertexPosTex; ///< Vertices Texture
uniform sampler1D orderTableTex; ///< Ternary Truth Table (TTT) Texture
uniform float vertTexSize; ///< Vertex (Quad) Texture width (same height)

/// Temporary variables
vec3 vertProj[4]; ///< Vertex positions in screen space
vec3 vertOrder[4]; ///< Vertices in right order (basis graph)
float scalarOrder[4]; ///< color in right order (basis graph)
float scalarOrig[4]; ///< Vertex original scalar value
ivec4 tests; ///< Classification tests: 4
float paramU1, paramU2; ///< Line x Line intersection parameters

/// Output data in FBOs
float centroidZ; ///< Centroid Z coordinate of the tetrahedron
int countTFan; ///< Count Triangle Fan (3, 4, 5 or 6 depend on the PT class)
int idTTT; ///< Index of Ternary Truth Table (TTT)
vec3 intersectionPoint; ///< Intersection point (already projected)
float thickness; ///< Distance between intersection point (projected and original)
float scalarFront, scalarBack; ///< Scalar front and back vertex

/// Data Retrieval
///   Read vertices information from tetrahedral texture, applying
///   transformations and computing centroid

void dataRetrieval(void) {

	centroidZ = 0.0;
	vec4 ids = texture2D(tetrahedralTex, gl_TexCoord[0].st).xyzw;
	vec4 texInfo, v;
	vec2 texParam;

	for (int i = 0; i < 4; ++i) { // for each vertex

		texParam = vec2((mod(ids[i],vertTexSize)+0.5)/vertTexSize,
				(floor(ids[i]/vertTexSize)+0.5)/vertTexSize);

		texInfo = texture2D(vertexPosTex, texParam).xyzw;

		scalarOrig[i] = texInfo.w;

		v = vec4(texInfo.xyz, 1.0);

		vertProj[i].z = (gl_ModelViewMatrix * v).z;

		centroidZ += vertProj[i].z;

		vertProj[i].xy = (gl_ModelViewProjectionMatrix * v).xy;

	}

	centroidZ *= 0.25;

}

/// Cross XY
///   Compute cross using only x and y coordinates
/// @arg a one vector
/// @arg b other vector
/// @return cross between a x b using only x and y

float crossXY(vec3 a, vec3 b) {

	return (a.x * b.y) - (a.y * b.x);

}

/// Project Tetrahedron Classification
///   Compute cross between tetrahedron vertices to classify
///   the projection using four classification tests

void ptClassification(void) {

	vec4 cross;

	vec3 vc1_0 = vertProj[1] - vertProj[0];
	vec3 vc2_0 = vertProj[2] - vertProj[0];
	vec3 vc3_0 = vertProj[3] - vertProj[0];
	vec3 vc1_2 = vertProj[1] - vertProj[2];
	vec3 vc1_3 = vertProj[1] - vertProj[3];

	cross.x = crossXY(vc1_0, vc2_0);
	cross.y = crossXY(vc1_0, vc3_0);
	cross.z = crossXY(vc2_0, vc3_0);
	cross.w = crossXY(vc1_2, vc1_3);

	countTFan = 5;

	for (int i = 0; i < 4; ++i) {

		tests[i] = int(sign( cross[i]) ) + 1;

		if (tests[i] == 1) --countTFan;

	}

	if (countTFan < 3) discard;

	idTTT = tests.x * 27 + tests.y * 9 + tests.z * 3 + tests.w * 1;

}

/// Order Vertices
///   Perform TTT texture lookup to order the tetrahedron vertices

void orderVertices(void) {

	ivec4 tableRow = ivec4( texture1D(orderTableTex, float(idTTT)/80.0).xyzw*4.0 + 0.5 );

	for(int i = 0; i < 4; i++) {

		if (tableRow[i] == 0) {

			vertOrder[i] = vertProj[0];
			scalarOrder[i] = scalarOrig[0];

		} else if (tableRow[i] == 1) {

			vertOrder[i] = vertProj[1];
			scalarOrder[i] = scalarOrig[1];

		} else if (tableRow[i] == 2) {

			vertOrder[i] = vertProj[2];
			scalarOrder[i] = scalarOrig[2];

		} else {

			vertOrder[i] = vertProj[3];
			scalarOrder[i] = scalarOrig[3];

		}

	}

}

/// Compute line intersection parameters

void computeParams(void) {

	float denominator, numeratorU1, numeratorU2;

	if (countTFan == 3) {

		paramU1 = 1.0;
		paramU2 = 1.0;

	} else if (countTFan == 4) {

		/// Line intersection denominator between v0->v2 and v1->v3
		denominator = ((vertOrder[3].y - vertOrder[1].y) * (vertOrder[2].x - vertOrder[0].x)) -
			((vertOrder[3].x - vertOrder[1].x) * (vertOrder[2].y - vertOrder[0].y));

		/// Line defined by vector v1->v3
		numeratorU2 = ((vertOrder[2].x - vertOrder[0].x) * (vertOrder[0].y - vertOrder[1].y)) -
			((vertOrder[2].y - vertOrder[0].y) * (vertOrder[0].x - vertOrder[1].x));

		paramU1 = 1.0;
		paramU2 = numeratorU2 / denominator;

	} else {
		/// Line intersection denominator between v0->v2 and v1->v3
		denominator = ((vertOrder[3].y - vertOrder[1].y) * (vertOrder[2].x - vertOrder[0].x)) -
			((vertOrder[3].x - vertOrder[1].x) * (vertOrder[2].y - vertOrder[0].y));

		/// Line defined by vector v0->v2
		numeratorU1 = ((vertOrder[3].x - vertOrder[1].x) * (vertOrder[0].y - vertOrder[1].y)) -
			((vertOrder[3].y - vertOrder[1].y) * (vertOrder[0].x - vertOrder[1].x));

		/// Line defined by vector v1->v3
		numeratorU2 = ((vertOrder[2].x - vertOrder[0].x) * (vertOrder[0].y - vertOrder[1].y)) -
			((vertOrder[2].y - vertOrder[0].y) * (vertOrder[0].x - vertOrder[1].x));

		paramU1 = numeratorU1 / denominator;
		paramU2 = numeratorU2 / denominator;

	}

}

/// Compute intersection between lines (using the basis graph)

void computeIntersection(void) {

	intersectionPoint = vec3(0.0);

	if (countTFan == 3) {

		thickness = (vertOrder[0].z - vertOrder[1].z);

	}  else if (countTFan == 4) {

		float zBackIntersection = vertOrder[1].z +
			paramU2*(vertOrder[3].z - vertOrder[1].z);

		thickness = (vertOrder[2].z - zBackIntersection);

	} else {

		/// Find z coordinate of back intersection point by
		///   interpolating original vertices (not projected)
		float zBackIntersection = vertOrder[1].z + paramU2*(vertOrder[3].z - vertOrder[1].z);
	
		/// Find ordered intersection point between the two ordered lines (basis graph)
		intersectionPoint = (vertOrder[0] + paramU1*(vertOrder[2] - vertOrder[0]));

		thickness = (intersectionPoint.z - zBackIntersection);

	}

}

/// Compute scalar front and back
///   Interpolate scalar values in the same manner as done for intersection vertex

void computeScalars(void) {

	if (countTFan == 6) {

		scalarFront = scalarOrder[0] + paramU1*(scalarOrder[2] - scalarOrder[0]);
		scalarBack = scalarOrder[1] + paramU2*(scalarOrder[3] - scalarOrder[1]);

	} else if (countTFan == 5) {

		scalarFront = scalarOrder[2];
		float tmp = scalarOrder[1] + paramU2*(scalarOrder[3] - scalarOrder[1]);
		scalarBack = scalarOrder[0] + (tmp - scalarOrder[0])*paramU1;

	} else if (countTFan == 4) {

		scalarFront = scalarOrder[2];
		scalarBack = scalarOrder[1] + paramU2*(scalarOrder[3] - scalarOrder[1]);

	} else {

		scalarFront = scalarOrder[0];
		scalarBack = scalarOrder[1];

	}

}

/// Main

void main(void) {

	dataRetrieval();

	ptClassification();

	orderVertices();

	computeParams();

	computeIntersection();

	if (countTFan == 5) {

		if (paramU1 > 1.0) {

			thickness /= paramU1;
			paramU1 = 1.0 / paramU1;

		} else { // count == 6 ; thick vertex == intersection vertex

			countTFan = 6;

		}

	}

	computeScalars();

	/// Output data in FBOs
	gl_FragData[0] = vec4( intersectionPoint.xy, centroidZ, idTTT );
	gl_FragData[1] = vec4( scalarFront, scalarBack, abs(thickness), countTFan );

}
