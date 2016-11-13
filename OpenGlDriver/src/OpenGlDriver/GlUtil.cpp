/*
    Copyright (c) 2011 Xeatheran Minexew

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#include "OpenGlDriver.hpp"

namespace OpenGlDriver
{
    // Source: our beloved Mesa3D (not being sarcastic here; usually I am when saying stuff like that)

    static void __gluMultMatricesd( const GLdouble a[16], const GLdouble b[16], GLdouble r[16] )
    {
        for ( int i = 0; i < 4; i++ )
            for ( int j = 0; j < 4; j++ )
                r[i * 4 + j] = a[i * 4 + 0] * b[0 * 4 + j] + a[i * 4 + 1] * b[1 * 4 + j] + a[i * 4 + 2] * b[2 * 4 + j] + a[i * 4 + 3] * b[3 * 4 + j];
    }

    int gluInvertMatrixd( const GLdouble m[16], GLdouble invOut[16] )
    {
        double inv[16], det;

        inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
                 + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
        inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
                 - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
        inv[8] =   m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
                 + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
        inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
                 - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
        inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
                 - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
        inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
                 + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
        inv[9] =  -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
                 - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
        inv[13] =  m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
                 + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
        inv[2] =   m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
                 + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
        inv[6] =  -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
                 - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
        inv[10] =  m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
                 + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
        inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
                 - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
        inv[3] =  -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
                 - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
        inv[7] =   m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
                 + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
        inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
                 - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
        inv[15] =  m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
                 + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

        det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];

        if ( det == 0 )
            return GL_FALSE;

        det = 1.0 / det;

        for ( int i = 0; i < 16; i++ )
            invOut[i] = inv[i] * det;

        return GL_TRUE;
    }

    void gluMultMatrixVecd( const GLdouble matrix[16], const GLdouble in[4], GLdouble out[4] )
    {
        for ( int i = 0; i < 4; i++ )
            out[i] = in[0] * matrix[0*4+i] + in[1] * matrix[1*4+i] + in[2] * matrix[2*4+i] + in[3] * matrix[3*4+i];
    }

    static void gluMakeIdentityf( GLfloat m[16] )
    {
        m[0+4*0] = 1; m[0+4*1] = 0; m[0+4*2] = 0; m[0+4*3] = 0;
        m[1+4*0] = 0; m[1+4*1] = 1; m[1+4*2] = 0; m[1+4*3] = 0;
        m[2+4*0] = 0; m[2+4*1] = 0; m[2+4*2] = 1; m[2+4*3] = 0;
        m[3+4*0] = 0; m[3+4*1] = 0; m[3+4*2] = 0; m[3+4*3] = 1;
    }

    void gluLookAt( GLdouble eyex, GLdouble eyey, GLdouble eyez, GLdouble centerx, GLdouble centery, GLdouble centerz, GLdouble upx, GLdouble upy, GLdouble upz )
    {
        GLfloat m[4][4];

        Vector<double> eye( eyex, eyey, eyez );
        Vector<double> center( centerx, centery, centerz );
        Vector<double> up( upx, upy, upz );

        Vector<float> forward = ( center - eye ).normalize();
        Vector<float> side = forward.crossProduct( up ).normalize();

        up = side.crossProduct( forward );

        gluMakeIdentityf( &m[0][0] );

        m[0][0] = side.x;
        m[1][0] = side.y;
        m[2][0] = side.z;

        m[0][1] = ( GLfloat ) up.x;
        m[1][1] = ( GLfloat ) up.y;
        m[2][1] = ( GLfloat ) up.z;

        m[0][2] = -forward.x;
        m[1][2] = -forward.y;
        m[2][2] = -forward.z;

        glMultMatrixf( &m[0][0] );
        glTranslated( -eye.x, -eye.y, -eye.z );
    }

    void gluPerspective( float fov, float aspect, float zNear, float zFar )
    {
        float fH = ( float )( zNear * tan( fov / 360.0f * M_PI ) );
        float fW = fH * aspect;

        glFrustum( -fW, fW, -fH, fH, zNear, zFar );
    }

    GLint gluUnProject( GLdouble winx, GLdouble winy, GLdouble winz, const GLdouble modelMatrix[16], const GLdouble projMatrix[16],
            const GLint viewport[4], GLdouble* objx, GLdouble* objy, GLdouble* objz )
    {
        double finalMatrix[16], in[4], out[4];

        __gluMultMatricesd( modelMatrix, projMatrix, finalMatrix );

        if ( !gluInvertMatrixd( finalMatrix, finalMatrix ) )
            return GL_FALSE;

        in[0] = winx;
        in[1] = winy;
        in[2] = winz;
        in[3] = 1.0;

        /* Map x and y from window coordinates */
        in[0] = ( in[0] - viewport[0] ) / viewport[2];
        in[1] = ( in[1] - viewport[1] ) / viewport[3];

        /* Map to range -1 to 1 */
        in[0] = in[0] * 2 - 1;
        in[1] = in[1] * 2 - 1;
        in[2] = in[2] * 2 - 1;

        gluMultMatrixVecd( finalMatrix, in, out );

        if ( out[3] == 0.0 )
            return GL_FALSE;

        out[0] /= out[3];
        out[1] /= out[3];
        out[2] /= out[3];
        *objx = out[0];
        *objy = out[1];
        *objz = out[2];

        return GL_TRUE;
    }

    GLint gluUnProject2( GLdouble winx, GLdouble winy, GLdouble winz, const GLdouble modelMatrix[16], const GLdouble projMatrix[16],
            const GLint viewport[4], GLdouble* objx, GLdouble* objy, GLdouble* objz )
    {
        double finalMatrix[16], in[4], out[4];

        __gluMultMatricesd( modelMatrix, projMatrix, finalMatrix );

        if ( !gluInvertMatrixd( finalMatrix, finalMatrix ) )
            return GL_FALSE;

        in[0] = winx;
        in[1] = winy;
        in[2] = winz;
        in[3] = 0.0;

        gluMultMatrixVecd( finalMatrix, in, out );

        *objx = out[0];
        *objy = out[1];
        *objz = out[2];

        return GL_TRUE;
    }
}
