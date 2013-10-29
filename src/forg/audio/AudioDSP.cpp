#include "forg_pch.h"
#include "AudioDSP.h"
#include "base.h"
#include "math/Math.h"

namespace forg { namespace audio { namespace dsp {
    /*
    void CalcBandpassCoeffs( float Fc, float Q, float gain, vector4_out coeffsA, vector4_out coeffsB )
    {
        // BPF:        H(s) = s / (s^2 + s/Q + 1)  (constant skirt gain, peak gain = Q)
        // 
        //                 b0 =   sin(w0)/2  =   Q*alpha
        //                 b1 =   0
        //                 b2 =  -sin(w0)/2  =  -Q*alpha
        //                 a0 =   1 + alpha
        //                 a1 =  -2*cos(w0)
        //                 a2 =   1 - alpha

        Q = ( float )native_fsel( Q, Q, .0001f );
        Fc = ( float )native_fsel( Fc, Fc, FLT_EPSILON * 2.0f );

        // Fc = (2 * sin(M_PI * CutoffFrequency / SampleRate));

        //     float w0 = ( float )M_PI * Fc;
        //     float sin0 = sin( w0 );
        //     float cos0 = cos( w0 );

        // its comparable to built-in xaudio2 filter if we take this:
        //float w0 = ( float )M_PI * Fc;
        float sin0 = Fc;
        float cos0 = sqrt(1.0f - sin0*sin0);

        float alpha = sin0 / ( 2.0f * Q );

        float a0 = alpha + 1.0f;

        coeffsA[0] = ( -2.0f * cos0 ) / a0;
        coeffsA[1] = ( 1.0f - alpha ) / a0;
        coeffsA[2] = 0.0f;
        coeffsA[3] = 0.0f;

        coeffsB[0] = gain * ( sin0 / 2.0f ) / a0;
        coeffsB[1] = 0.0f;
        coeffsB[2] = gain * ( -sin0 / 2.0f ) / a0;
        coeffsB[3] = 0.0f;
    }

    void CalcHighpassCoeffs( float Fc, float Q, vector4_out coeffsA, vector4_out coeffsB )
    {
        Q = (float)native_fsel( Q, Q, .0001f );
        Fc = (float)native_fsel( Fc, Fc, FLT_EPSILON * 2.0f );

        float w0 = (float)M_PI*Fc;
        float sin0 = sin(w0);
        float cos0 = cos(w0);

        float alpha = sin0/(2.0f*Q);

        float a0 = alpha + 1.0f;
        coeffsA[0] =( -2.0f * cos0)/a0;
        coeffsA[1] = (1.0f - alpha)/a0;
        coeffsA[2] = 0.0f;
        coeffsA[3] = 0.0f;

        coeffsB[0] = ((1.0f + cos0)/2.0f)/a0;
        coeffsB[1] = (-(1.0f + cos0))/a0;
        coeffsB[2] = ((1.0f + cos0)/2.0f)/a0;
        coeffsB[3] = 0.0f;
    }
    */

    /// return B if A not negative, else C
    float native_fsel(float A, float B, float C)
    {
        return (A >= 0.0f ? B : C);
    }

    //--------------------------------------------------------------------------------------
    // Filter
    //
    // This is a standard "Direct Form 1" biquad filter implementation, which can be
    // used to represent several different types of filter depending on the coefficients
    // used. Helpers to generate a few different filter types are included. 
    // 
    // Biquad filters are IIR, or Infinite Impulse Response, designs. In practice this 
    // means that the filter uses a feedback mechanism: each calculation requires not
    // only an input sample but also a number of previous samples from both the input
    // and the output. The particular design used here is optimal because it requires 
    // exactly four output samples, which means that the output can be recycled one 
    // vector at a time.

    //
    // y[n] = (b0/a0)*x[n] + (b1/a0)*x[n-1] + (b2/a0)*x[n-2]
    //              0*y[n] - (a1/a0)*y[n-1] - (a2/a0)*y[n-2]            (Eq 4)
    //
    // w0 = 2*pi*f0/Fs
    // 
    // cos(w0)
    // sin(w0)
    // 
    // alpha = sin(w0)/(2*Q)                                       (case: Q)
    // = sin(w0)*sinh( ln(2)/2 * BW * w0/sin(w0) )           (case: BW)
    // = sin(w0)/2 * sqrt( (A + 1/A)*(1/S - 1) + 2 )         (case: S)
    // 
    // FYI: The relationship between bandwidth and Q is
    //      1/Q = 2*sinh(ln(2)/2*BW*w0/sin(w0))     (digital filter w BLT)
    //      or   1/Q = 2*sinh(ln(2)/2*BW)             (analog filter prototype)
    // 
    //      The relationship between shelf slope and Q is
    //      1/Q = sqrt((A + 1/A)*(1/S - 1) + 2)
    // 
    //      2*sqrt(A)*alpha  =  sin(w0) * sqrt( (A^2 + 1)*(1/S - 1) + 2*A )
    //      is a handy intermediate variable for shelving EQ filters.

    void __forceinline FilterCore( 
    const float4&  input,
    float4& output,
    const float4& prevInput, 
    const float4& prevOutput,
    const float4& coeffA,
    const float4& coeffB,
    uint32 count = 4)
    {
        float4 A = coeffA; //.wzyx();
        float4 B = coeffB; //.wzyx();

        // y[n] = (b0/a0)*x[n] + (b1/a0)*x[n-1] + (b2/a0)*x[n-2]
        //                     - (a1/a0)*y[n-1] - (a2/a0)*y[n-2]            (Eq 4)

        // y = B * X - A * Y

        // all coefficients are divided by a0 in CalcBandPassCoefficients

        float input_all[7] = {prevInput[1], prevInput[2], prevInput[3], input[0], input[1], input[2], input[3]};
        float output_all[8] = {prevOutput[0], prevOutput[1], prevOutput[2], prevOutput[3], 0.0f, 0.0f, 0.0f, 0.0f};

        float4 xValues;
        float4 yValues;

        for (uint32 i=0; i<count; i++)
        {
            xValues.set(input_all[i], input_all[i+1], input_all[i+2], input_all[i+3]);
            yValues.set(output_all[i], output_all[i+1], output_all[i+2], output_all[i+3]);

            output_all[4 + i] = dot(xValues, B) - dot(yValues, A);
        }

        output.set(output_all[4], output_all[5], output_all[6], output_all[7]);
    }    

    void CalcLowpassCoeffs( float Fc, float Q, float4& coeffsA, float4& coeffsB )
    {
        Q = ( float )native_fsel( Q, Q, .0001f );
        Fc = ( float )native_fsel( Fc, Fc, FLT_EPSILON * 2.0f );

        float w0 = ( float )forg::math::Math::PI * Fc;
        float sin0 = sin( w0 );
        float cos0 = cos( w0 );

        float alpha = sin0 / ( 2.0f * Q );

        float a0 = alpha + 1.0f;
        coeffsA[0] = ( -2.0f * cos0 ) / a0;
        coeffsA[1] = ( 1.0f - alpha ) / a0;
        coeffsA[2] = 0.0f;
        coeffsA[3] = 0.0f;

        coeffsB[0] = ( ( 1.0f - cos0 ) / 2.0f ) / a0;
        coeffsB[1] = ( 1.0f - cos0 ) / a0;
        coeffsB[2] = ( ( 1.0f - cos0 ) / 2.0f ) / a0;
        coeffsB[3] = 0.0f;
    }

}}}

