#include <cassert>
#include <cmath>

#include "BilateralFilter.h"

// =========================
//  Public Member Functions
// =========================

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

	if (m_KernelSize <= 0) 
		m_Radius = m_RangeSigma * 1.5;
	else 
		m_Radius = m_KernelSize / 2;

	m_Radius = (m_Radius >= 1) ? m_Radius : 1;
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
	for (unsigned int i = 0; i < apprRes; ++i) {
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

bool 
BilateralFilter::Filter(const double * src, double * dest, int height, int width, int channel)
{
	if (src == NULL || dest == NULL) {
		return false;
	}

	assert (channel == 1 || channel == 3);
	assert (height >= 1 && width >= 1);

	int newH = height + 2 * m_Radius, newW = width + 2 * m_Radius;
	double * exSrc = new double[newH * newW * channel];
	memset(exSrc, 0, newH * newW * channel * sizeof(double));

	for (int i = m_Radius; i + m_Radius < newH; ++i)
	for (int j = m_Radius; j + m_Radius < newW; ++j) {
		exSrc[i * newW + j] = src[(i - m_Radius) * width + (j - m_Radius)];
	}

	assert (m_Radius >= 1 && m_Radius < width && m_Radius < height);

	/*
	for (int i = m_Radius; i + m_Radius < newH; ++i) 
	{
		for (int j = 0; j < m_Radius; ++j) {
			exSrc[i * newW + j] = exSrc[i * newW + (m_Radius - 1 - j + m_Radius)];
		}
		for (int j = width; j < newW; ++j) {
			exSrc[i * newW + j] = exSrc[i * newW + (width - 1 - j + width)];
		}
	}

	for (int j = 0; j < newW; ++j) 
	{
		for (int i = 0; i < m_Radius; ++i) {
			exSrc[i * newW + j] = exSrc[(m_Radius - 1 - i + m_Radius) * newW + j];
		}
		for (int i = height; i < newH; ++i) {
			exSrc[i * newW + j] = exSrc[(height - 1 - i + height) * newW + j];
		}
	}
	*/

	m_SpatialOffset.clear();
	
	for (int i = -m_Radius; i <= m_Radius; ++i) 
	for (int j = -m_Radius; j <= m_Radius; ++j) 
	{
		double r = sqrt((double)(i*i) + (double)(j*j));
		if (r > m_Radius)
			continue;
	
		m_SpatialOffset.push_back(i * newW + j);
	}

	assert (m_SpatialKernel.size() == m_SpatialOffset.size());

	for (int i = m_Radius; i + m_Radius < newH; ++i) 
	for (int j = m_Radius; j + m_Radius < newW; ++j) 
	// for (int c = 0; c < channel; ++c)
	{
		double sum1 = 0.0f, sum2 = 0.0f;
		double v_ij = exSrc[i * newW + j];

		for (int k = 0; k < m_SpatialKernel.size(); ++k) 
		{
			double v_k = exSrc[i * newW + j + m_SpatialOffset[k]];
			
			double f1 = m_SpatialKernel[k];
			double f2 = m_RangeKernel[(int)(abs(v_k - v_ij))];

			sum1 += f1 * f2 * v_k;
			sum2 += f1 * f2;
		}

		dest[(i - m_Radius) * width + (j - m_Radius)] = sum1 / sum2;
	}

	delete [] exSrc;

	return true;
}