#include <cassert>
#include <cmath>

#include "BilateralFilter.h"

// ===================
//  Public Member Functions
// ===================

BilateralFilter::
BilateralFilter(double spatialSigma, double rangeSigma, int kernelSize, int apprRes) : 
	m_SpatialSigma(spatialSigma),
	m_RangeSigma(rangeSigma),
	m_KernelSize(kernelSize),
	m_ApproxResolution(apprRes)
{
	if (m_ApproxResolution <= 0)
		m_ApproxResolution = 256;

	if (m_SpatialSigma <= 0) 
		m_SpatialSigma = 1.f;
	if (m_RangeSigma <= 0)
		m_RangeSigma = 1.f;

	double gauss_color_coeff = -0.5f / (m_RangeSigma * m_RangeSigma);
	double gauss_space_coeff = -0.5f / (m_SpatialSigma * m_SpatialSigma);

	// int radius;
	if (m_KernelSize <= 0) 
		m_Radius = m_RangeSigma * 1.5;
	else 
		m_Radius = m_KernelSize / 2;

	m_Radius = (m_Radius > 1) ? m_Radius : 1;
	m_KernelSize = 2 * m_Radius + 1;

	m_SpatialKernel.clear();
	m_RangeKernel.clear();

	double value;

	//   Initialize Spatial Filter	
	for (int i = -m_Radius; i <= m_Radius; ++i) 
	for (int j = -m_Radius; j <= m_Radius; ++j) 
	{
		double r = sqrt((double)(i*i) + (double)(j*j));
		if (r > m_Radius)
			continue;

		value = exp (r * r * gauss_space_coeff);
		m_SpatialKernel.push_back(value);
	}

	//   Initialize Range Filter
	for (unsigned int i = 0; i < apprRes; ++i) 
	{
		// double frac = (double)(i) / (double)(apprRes);
		// value = exp (-(frac * frac) / (2 * m_RangeSigma * m_RangeSigma));
		value = exp (i * i * gauss_color_coeff);
		m_RangeKernel.push_back(value);
	}
}

BilateralFilter::
~BilateralFilter()
{
}

// 
// Bilateral Filtering
// 

bool BilateralFilter::Filter(const double * src, double * dest, int height, int width, int channel)
{
	if (src == NULL || dest == NULL) {
		return false;
	}

	assert (channel == 1 || channel == 3);
	assert (height >= 1 && width >= 1);

	m_SpatialOffset.clear();
	
	for (int i = -m_Radius; i <= m_Radius; ++i) 
	for (int j = -m_Radius; j <= m_Radius; ++j) 
	{
		double r = sqrt((double)(i*i) + (double)(j*j));
		if (r > m_Radius)
			continue;
	
		m_SpatialOffset.push_back(i * width + j);
	}

	assert (m_SpatialKernel.size() == m_SpatialOffset.size());

	double * tmp = new double[height * width * channel];
	memcpy(tmp, src, height * width * channel * sizeof(double));

	for (int i = m_Radius; i + m_Radius < height; ++i) 
	for (int j = m_Radius; j + m_Radius < width; ++j) 
	// for (int c = 0; c < channel; ++c)
	{
		double sum1 = 0.0f, sum2 = 0.0f;
		double v_ij = src[i * width + j];
		// double v_ij = src[channel * (i * width + j) + c];

		for (int k = 0; k < m_SpatialKernel.size(); ++k) 
		{
			double v_k = src[i * width + j + m_SpatialOffset[k]];
			
			double f1 = m_SpatialKernel[k];
			double f2 = m_RangeKernel[(int)(abs(v_k - v_ij))];

			sum1 += f1 * f2 * v_k;
			sum2 += f1 * f2;
		}

		/*
		for (int deltaH = -m_KernelSize; deltaH <= m_KernelSize; ++deltaH) 
		for (int deltaW = -m_KernelSize; deltaW <= m_KernelSize; ++deltaW) 
		{
			double v_mk = src[(i + deltaH) * width + (j + deltaW)]; 
				
			double f1 = m_SpatialKernel[(m_KernelSize + deltaH) * (2 * m_KernelSize + 1) + (m_KernelSize + deltaW)];
			double f2 = m_RangeKernel[(int)(abs(v_mk - v_ij))];

			sum1 += f1 * f2 * v_mk;
			sum2 += f1 * f2;
		}*/

		tmp[i * width + j] = sum1 / sum2;
	}

	memcpy(dest, tmp, height * width * channel * sizeof(double));
	delete [] tmp;

	return true;

}