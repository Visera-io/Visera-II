/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the 
"Apache License"); you may not use this file except in compliance with the 
Apache License. You may obtain a copy of the Apache License at 
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

  Copyright (c) 2025 Audiokinetic Inc.
*******************************************************************************/

// AkSpeakerVolumes.h

/// \file 
/// Multi-channel volume definitions and services.
/// Always associated with an AkChannelConfig. In the case of standard configurations, the volume items ordering
/// match the bit ordering in the channel mask, except for the LFE which is skipped and placed at the end of the
/// volume array.
/// See \ref goingfurther_speakermatrixcallback for an example of how to manipulate speaker volume vectors/matrices.

#ifndef _AK_SPEAKER_VOLUMES_H_
#define _AK_SPEAKER_VOLUMES_H_

#include <string.h>
#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Platforms/Generic/AkSpeakerVolumes.h>

/// Copy volumes.
AkForceInline void AK_SpeakerVolumes_Vector_Copy(AkSpeakerVolumesVectorPtr in_pVolumesDst, AkSpeakerVolumesConstVectorPtr in_pVolumesSrc, AkUInt32 in_uNumChannels)
{
	if (in_uNumChannels)
		memcpy(in_pVolumesDst, in_pVolumesSrc, in_uNumChannels * sizeof(AkReal32));
}

/// Copy volumes with gain.
AkForceInline void AK_SpeakerVolumes_Vector_CopyAndApplyGain(AkSpeakerVolumesVectorPtr in_pVolumesDst, AkSpeakerVolumesConstVectorPtr in_pVolumesSrc, AkUInt32 in_uNumChannels, AkReal32 in_fGain)
{
	for (AkUInt32 uChan = 0; uChan < in_uNumChannels; uChan++)
	{
		in_pVolumesDst[uChan] = in_pVolumesSrc[uChan] * in_fGain;
	}
}

/// Clear volumes.
AkForceInline void AK_SpeakerVolumes_Vector_Zero(AkSpeakerVolumesVectorPtr in_pVolumes, AkUInt32 in_uNumChannels)
{
	if (in_uNumChannels)
		memset(in_pVolumes, 0, in_uNumChannels * sizeof(AkReal32));
}

/// Accumulate two volume vectors.
AkForceInline void AK_SpeakerVolumes_Vector_Add(AkSpeakerVolumesVectorPtr io_pVolumesDst, AkSpeakerVolumesConstVectorPtr in_pVolumesSrc, AkUInt32 in_uNumChannels)
{
	for (AkUInt32 uChan = 0; uChan < in_uNumChannels; uChan++)
	{
		io_pVolumesDst[uChan] += in_pVolumesSrc[uChan];
	}
}

/// Compute the sum of all components of a volume vector.
AkForceInline AkReal32 AK_SpeakerVolumes_Vector_L1Norm(AkSpeakerVolumesConstVectorPtr in_pVolumes, AkUInt32 in_uNumChannels)
{
	AkReal32 total = 0;
	for (AkUInt32 uChan = 0; uChan < in_uNumChannels; uChan++)
	{
		total += in_pVolumes[uChan];
	}

	return total;
}

/// Multiply volume vector with a scalar.
AkForceInline void AK_SpeakerVolumes_Vector_MulScalar(AkSpeakerVolumesVectorPtr in_pVolumesDst, AkReal32 in_fVol, AkUInt32 in_uNumChannels)
{
	for (AkUInt32 uChan = 0; uChan < in_uNumChannels; uChan++)
	{
		in_pVolumesDst[uChan] *= in_fVol;
	}
}

/// Multiply two volume vectors.
AkForceInline void AK_SpeakerVolumes_Vector_Mul(AkSpeakerVolumesVectorPtr in_pVolumesDst, AkSpeakerVolumesConstVectorPtr in_pVolumesSrc, AkUInt32 in_uNumChannels)
{
	for (AkUInt32 uChan = 0; uChan < in_uNumChannels; uChan++)
	{
		in_pVolumesDst[uChan] *= in_pVolumesSrc[uChan];
	}
}

/// Get max for all elements of two volume vectors, independently.
AkForceInline void AK_SpeakerVolumes_Vector_Max(AkSpeakerVolumesVectorPtr in_pVolumesDst, AkSpeakerVolumesConstVectorPtr in_pVolumesSrc, AkUInt32 in_uNumChannels)
{
	for (AkUInt32 uChan = 0; uChan < in_uNumChannels; uChan++)
	{
		in_pVolumesDst[uChan] = AkMax(in_pVolumesDst[uChan], in_pVolumesSrc[uChan]);
	}
}

/// Get min for all elements of two volume vectors, independently.
AkForceInline void AK_SpeakerVolumes_Vector_Min(AkSpeakerVolumesVectorPtr in_pVolumesDst, AkSpeakerVolumesConstVectorPtr in_pVolumesSrc, AkUInt32 in_uNumChannels)
{
	for (AkUInt32 uChan = 0; uChan < in_uNumChannels; uChan++)
	{
		in_pVolumesDst[uChan] = AkMin(in_pVolumesDst[uChan], in_pVolumesSrc[uChan]);
	}
}

/// Compute size (in bytes) required for given channel configurations.
AkForceInline AkUInt32 AK_SpeakerVolumes_Matrix_GetRequiredSize(AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut)
{
	return in_uNumChannelsIn * AK_SpeakerVolumes_Vector_GetRequiredSize(in_uNumChannelsOut);
}

/// Compute size (in number of elements) required for given channel configurations.
AkForceInline AkUInt32 AK_SpeakerVolumes_Matrix_GetNumElements(AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut)
{
	return in_uNumChannelsIn * AK_SpeakerVolumes_Vector_GetNumElements(in_uNumChannelsOut);
}

/// Get pointer to volume distribution for input channel in_uIdxChannelIn.
AkForceInline AkSpeakerVolumesVectorPtr AK_SpeakerVolumes_Matrix_GetChannel(AkSpeakerVolumesMatrixPtr in_pVolumeMx, AkUInt32 in_uIdxChannelIn, AkUInt32 in_uNumChannelsOut)
{
	return in_pVolumeMx + in_uIdxChannelIn * AK_SpeakerVolumes_Vector_GetNumElements(in_uNumChannelsOut);
}
AkForceInline AkSpeakerVolumesConstVectorPtr AK_SpeakerVolumes_Matrix_GetChannel_Const(AkSpeakerVolumesConstMatrixPtr in_pVolumeMx, AkUInt32 in_uIdxChannelIn, AkUInt32 in_uNumChannelsOut)
{
	return in_pVolumeMx + in_uIdxChannelIn * AK_SpeakerVolumes_Vector_GetNumElements(in_uNumChannelsOut);
}

/// Copy matrix.
AkForceInline void AK_SpeakerVolumes_Matrix_Copy(AkSpeakerVolumesMatrixPtr in_pVolumesDst, AkSpeakerVolumesConstMatrixPtr in_pVolumesSrc, AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut)
{
	AkUInt32 uNumElements = AK_SpeakerVolumes_Matrix_GetNumElements(in_uNumChannelsIn, in_uNumChannelsOut);
	if (uNumElements)
		memcpy(in_pVolumesDst, in_pVolumesSrc, uNumElements * sizeof(AkReal32));
}

/// Copy matrix with gain.
AkForceInline void AK_SpeakerVolumes_Matrix_CopyAndApplyGain(AkSpeakerVolumesMatrixPtr in_pVolumesDst, AkSpeakerVolumesConstMatrixPtr in_pVolumesSrc, AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut, AkReal32 in_fGain)
{
	AkUInt32 uNumElements = AK_SpeakerVolumes_Matrix_GetNumElements(in_uNumChannelsIn, in_uNumChannelsOut);
	for (AkUInt32 uChan = 0; uChan < uNumElements; uChan++)
	{
		in_pVolumesDst[uChan] = in_pVolumesSrc[uChan] * in_fGain;
	}
}

/// Clear matrix.
AkForceInline void AK_SpeakerVolumes_Matrix_Zero(AkSpeakerVolumesMatrixPtr in_pVolumes, AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut)
{
	AkUInt32 uNumElements = AK_SpeakerVolumes_Matrix_GetNumElements(in_uNumChannelsIn, in_uNumChannelsOut);
	if (uNumElements)
		memset(in_pVolumes, 0, uNumElements * sizeof(AkReal32));
}

/// Multiply a matrix with a scalar.
AkForceInline void AK_SpeakerVolumes_Matrix_Mul(AkSpeakerVolumesMatrixPtr in_pVolumesDst, const AkReal32 in_fVol, AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut)
{
	AkUInt32 uNumElements = AK_SpeakerVolumes_Matrix_GetNumElements(in_uNumChannelsIn, in_uNumChannelsOut);
	for (AkUInt32 uChan = 0; uChan < uNumElements; uChan++)
	{
		in_pVolumesDst[uChan] *= in_fVol;
	}
}

/// Add all elements of two volume matrices, independently.
AkForceInline void AK_SpeakerVolumes_Matrix_Add(AkSpeakerVolumesMatrixPtr in_pVolumesDst, AkSpeakerVolumesConstMatrixPtr in_pVolumesSrc, AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut)
{
	AkUInt32 uNumElements = AK_SpeakerVolumes_Matrix_GetNumElements(in_uNumChannelsIn, in_uNumChannelsOut);
	for (AkUInt32 uChan = 0; uChan < uNumElements; uChan++)
	{
		in_pVolumesDst[uChan] += in_pVolumesSrc[uChan];
	}
}

/// Pointwise Multiply-Add of all elements of two volume matrices.
AkForceInline void AK_SpeakerVolumes_Matrix_MAdd(AkSpeakerVolumesMatrixPtr in_pVolumesDst, AkSpeakerVolumesConstMatrixPtr in_pVolumesSrc, AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut, AkReal32 in_fGain)
{
	AkUInt32 uNumElements = AK_SpeakerVolumes_Matrix_GetNumElements(in_uNumChannelsIn, in_uNumChannelsOut);
	for (AkUInt32 uChan = 0; uChan < uNumElements; uChan++)
	{
		in_pVolumesDst[uChan] += in_pVolumesSrc[uChan] * in_fGain;
	}
}

/// Get absolute max for all elements of two volume matrices, independently.
AkForceInline void AK_SpeakerVolumes_Matrix_AbsMax(AkSpeakerVolumesMatrixPtr in_pVolumesDst, AkSpeakerVolumesConstMatrixPtr in_pVolumesSrc, AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut)
{
	AkUInt32 uNumElements = AK_SpeakerVolumes_Matrix_GetNumElements(in_uNumChannelsIn, in_uNumChannelsOut);
	for (AkUInt32 uChan = 0; uChan < uNumElements; uChan++)
	{
		in_pVolumesDst[uChan] = ((in_pVolumesDst[uChan] * in_pVolumesDst[uChan]) > (in_pVolumesSrc[uChan] * in_pVolumesSrc[uChan])) ? in_pVolumesDst[uChan] : in_pVolumesSrc[uChan];
}
		}

/// Get max for all elements of two volume matrices, independently.
AkForceInline void AK_SpeakerVolumes_Matrix_Max(AkSpeakerVolumesMatrixPtr in_pVolumesDst, AkSpeakerVolumesConstMatrixPtr in_pVolumesSrc, AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut)
{
	AkUInt32 uNumElements = AK_SpeakerVolumes_Matrix_GetNumElements(in_uNumChannelsIn, in_uNumChannelsOut);
	for (AkUInt32 uChan = 0; uChan < uNumElements; uChan++)
	{
		in_pVolumesDst[uChan] = (in_pVolumesDst[uChan] > in_pVolumesSrc[uChan]) ? in_pVolumesDst[uChan] : in_pVolumesSrc[uChan];
	}
}

#ifdef __cplusplus
namespace AK
{
/// Multi-channel volume definitions and services.
namespace SpeakerVolumes
{
	/// Volume vector services.
	namespace Vector
	{
		AkForceInline void Copy(AkSpeakerVolumesVectorPtr in_pVolumesDst, AkSpeakerVolumesConstVectorPtr in_pVolumesSrc, AkUInt32 in_uNumChannels)
		{
			return AK_SpeakerVolumes_Vector_Copy(in_pVolumesDst, in_pVolumesSrc, in_uNumChannels);
		}
		AkForceInline void Copy(AkSpeakerVolumesVectorPtr in_pVolumesDst, AkSpeakerVolumesConstVectorPtr in_pVolumesSrc, AkUInt32 in_uNumChannels, AkReal32 in_fGain)
		{
			return AK_SpeakerVolumes_Vector_CopyAndApplyGain(in_pVolumesDst, in_pVolumesSrc, in_uNumChannels, in_fGain);
		}
		AkForceInline void Zero(AkSpeakerVolumesVectorPtr in_pVolumes, AkUInt32 in_uNumChannels)
		{
			return AK_SpeakerVolumes_Vector_Zero(in_pVolumes, in_uNumChannels);
		}
		AkForceInline void Add(AkSpeakerVolumesVectorPtr io_pVolumesDst, AkSpeakerVolumesConstVectorPtr in_pVolumesSrc, AkUInt32 in_uNumChannels)
		{
			return AK_SpeakerVolumes_Vector_Add(io_pVolumesDst, in_pVolumesSrc, in_uNumChannels);
		}
		AkForceInline AkReal32 L1Norm(AkSpeakerVolumesConstVectorPtr in_pVolumes, AkUInt32 in_uNumChannels)
		{
			return AK_SpeakerVolumes_Vector_L1Norm(in_pVolumes, in_uNumChannels);
		}
		AkForceInline void Mul( VectorPtr in_pVolumesDst, const AkReal32 in_fVol, AkUInt32 in_uNumChannels )
		{
			return AK_SpeakerVolumes_Vector_MulScalar(in_pVolumesDst, in_fVol, in_uNumChannels);
		}
		AkForceInline void Mul( VectorPtr in_pVolumesDst, ConstVectorPtr in_pVolumesSrc, AkUInt32 in_uNumChannels )
		{
			return AK_SpeakerVolumes_Vector_Mul(in_pVolumesDst, in_pVolumesSrc, in_uNumChannels);
		}
		AkForceInline void Max(AkSpeakerVolumesVectorPtr in_pVolumesDst, AkSpeakerVolumesConstVectorPtr in_pVolumesSrc, AkUInt32 in_uNumChannels)
		{
			return AK_SpeakerVolumes_Vector_Max(in_pVolumesDst, in_pVolumesSrc, in_uNumChannels);
		}
		AkForceInline void Min(AkSpeakerVolumesVectorPtr in_pVolumesDst, AkSpeakerVolumesConstVectorPtr in_pVolumesSrc, AkUInt32 in_uNumChannels)
		{
			return AK_SpeakerVolumes_Vector_Min(in_pVolumesDst, in_pVolumesSrc, in_uNumChannels);
		}
	}

	/// Volume matrix (multi-in/multi-out channel configurations) services.
	namespace Matrix
	{
		AkForceInline AkUInt32 GetRequiredSize(AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut)
		{
			return AK_SpeakerVolumes_Matrix_GetRequiredSize(in_uNumChannelsIn, in_uNumChannelsOut);
		}
		AkForceInline AkUInt32 GetNumElements(AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut)
		{
			return AK_SpeakerVolumes_Matrix_GetNumElements(in_uNumChannelsIn, in_uNumChannelsOut);
		}
		AkForceInline VectorPtr GetChannel( MatrixPtr in_pVolumeMx, AkUInt32 in_uIdxChannelIn, AkUInt32 in_uNumChannelsOut ) 
		{
			return AK_SpeakerVolumes_Matrix_GetChannel(in_pVolumeMx, in_uIdxChannelIn, in_uNumChannelsOut);
		}
		AkForceInline ConstVectorPtr GetChannel( ConstMatrixPtr in_pVolumeMx, AkUInt32 in_uIdxChannelIn, AkUInt32 in_uNumChannelsOut ) 
		{
			return AK_SpeakerVolumes_Matrix_GetChannel_Const(in_pVolumeMx, in_uIdxChannelIn, in_uNumChannelsOut);
		}
		AkForceInline void Copy( MatrixPtr in_pVolumesDst, ConstMatrixPtr in_pVolumesSrc, AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut )
		{
			return AK_SpeakerVolumes_Matrix_Copy(in_pVolumesDst, in_pVolumesSrc, in_uNumChannelsIn, in_uNumChannelsOut);
		}
		AkForceInline void Copy( MatrixPtr in_pVolumesDst, ConstMatrixPtr in_pVolumesSrc, AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut, AkReal32 in_fGain )
		{
			return AK_SpeakerVolumes_Matrix_CopyAndApplyGain(in_pVolumesDst, in_pVolumesSrc, in_uNumChannelsIn, in_uNumChannelsOut, in_fGain);
		}
		AkForceInline void Zero(AkSpeakerVolumesMatrixPtr in_pVolumes, AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut)
		{
			return AK_SpeakerVolumes_Matrix_Zero(in_pVolumes, in_uNumChannelsIn, in_uNumChannelsOut);
		}
		AkForceInline void Mul(AkSpeakerVolumesMatrixPtr in_pVolumesDst, const AkReal32 in_fVol, AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut)
		{
			return AK_SpeakerVolumes_Matrix_Mul(in_pVolumesDst, in_fVol, in_uNumChannelsIn, in_uNumChannelsOut);
		}
		AkForceInline void Add(AkSpeakerVolumesMatrixPtr in_pVolumesDst, AkSpeakerVolumesConstMatrixPtr in_pVolumesSrc, AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut)
		{
			return AK_SpeakerVolumes_Matrix_Add(in_pVolumesDst, in_pVolumesSrc, in_uNumChannelsIn, in_uNumChannelsOut);
		}
		AkForceInline void MAdd(AkSpeakerVolumesMatrixPtr in_pVolumesDst, AkSpeakerVolumesConstMatrixPtr in_pVolumesSrc, AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut, AkReal32 in_fGain)
		{
			return AK_SpeakerVolumes_Matrix_MAdd(in_pVolumesDst, in_pVolumesSrc, in_uNumChannelsIn, in_uNumChannelsOut, in_fGain);
		}
		AkForceInline void AbsMax(AkSpeakerVolumesMatrixPtr in_pVolumesDst, AkSpeakerVolumesConstMatrixPtr in_pVolumesSrc, AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut)
		{
			return AK_SpeakerVolumes_Matrix_AbsMax(in_pVolumesDst, in_pVolumesSrc, in_uNumChannelsIn, in_uNumChannelsOut);
		}
		AkForceInline void Max(AkSpeakerVolumesMatrixPtr in_pVolumesDst, AkSpeakerVolumesConstMatrixPtr in_pVolumesSrc, AkUInt32 in_uNumChannelsIn, AkUInt32 in_uNumChannelsOut)
		{
			return AK_SpeakerVolumes_Matrix_Max(in_pVolumesDst, in_pVolumesSrc, in_uNumChannelsIn, in_uNumChannelsOut);
		}
	}
}
}
#endif // __cplusplus

#endif  //_AK_SPEAKER_VOLUMES_H_
