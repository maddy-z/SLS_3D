#include <cassert>
#include <cmath>

#include "BilateralFilter.h"

// ===================
//  Public Member Functions
// ===================

BilateralFilter::
BilateralFilter(double spatialSigma, double rangeSigma, unsigned int kernelSize, unsigned int apprRes) : 
	m_SpatialSigma(spatialSigma),
	m_RangeSigma(rangeSigma),
	m_KernelSize(kernelSize),
	m_ApproxResultion(apprRes)
{

	m_SpatialKernel.clear();
	m_RangeKernel.clear();

	double value;
	double dx, dy;

	int kernelHeight, kernelWidth;
	kernelHeight = kernelWidth = 2 * kernelSize + 1;

	// ===============
	//   Initialize Spatial Filter
	// ===============
	
	for (int i = 0; i < kernelHeight; ++i) 
	{
		for (int j = 0; j < kernelWidth; ++j) 
		{
			dx = (i - kernelSize);
			dy = (j - kernelSize);
			value = exp (-(dx * dx + dy * dy) / (2 * m_SpatialSigma * m_SpatialSigma));
	
			m_SpatialKernel.push_back(value);
		}
	}

	// ===============
	//   Initialize Range Filter
	// ===============

	for (unsigned int i = 0; i <= apprRes; ++i) 
	{
		double frac = (double)(i) / (double)(apprRes);
		value = exp (-(frac * frac) / (2 * m_RangeSigma * m_RangeSigma));

		m_RangeKernel.push_back(value);
	}

	return;
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

	double * tmp = new double[height * width * channel];
	memcpy(tmp, src, height * width * channel * sizeof(double));

	for (int i = m_KernelSize; i + m_KernelSize < height; ++i) 
	for (int j = m_KernelSize; j + m_KernelSize < width; ++j) 
	// for (int c = 0; c < channel; ++c)
	{
		double sum1 = 0.0f, sum2 = 0.0f;
		double v_ij = src[i * width + j];
		// double v_ij = src[channel * (i * width + j) + c];

		for (int deltaH = -m_KernelSize; deltaH <= m_KernelSize; ++deltaH) 
		for (int deltaW = -m_KernelSize; deltaW <= m_KernelSize; ++deltaW) 
		{
			double v_mk = src[(i + deltaH) * width + (j + deltaW)]; 
				
			double f1 = m_SpatialKernel[(m_KernelSize + deltaH) * (2 * m_KernelSize + 1) + (m_KernelSize + deltaW)];
			double f2 = m_RangeKernel[(int)(abs(v_mk - v_ij))];

			sum1 += f1 * f2 * v_mk;
			sum2 += f1 * f2;
		}

		// if (sum2 >= 1.e-11) {
			tmp[i * width + j] = sum1 / sum2;
		// }
	}

	memcpy(dest, tmp, height * width * channel * sizeof(double));
	delete [] tmp;

	return true;

}