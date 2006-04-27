#ifndef _SLS_BILATERALFILTER_H_
#define _SLS_BILATERALFILTER_H_

#include <vector>

class BilateralFilter
{

public:

	BilateralFilter(double spatialSigma, double rangeSigma, unsigned int kernelSize, unsigned int apprRes);
	virtual ~BilateralFilter();

	bool Filter(const double * src, double * dest, int height, int width, int channel);

	void printFilterInfo() const
	{
		printf("Spatial Sigma = %f\n", m_SpatialSigma);
		printf("Range Sigma= %f\n", m_RangeSigma);
		printf("Kernel Size = %u\n", m_KernelSize);
		printf("Appr Resolution = %u\n", m_ApproxResultion);
	}

private:

	double m_SpatialSigma;
	double m_RangeSigma;

	unsigned int m_KernelSize;
	unsigned int m_ApproxResultion;

	std::vector<double> m_SpatialKernel;
	std::vector<double>	m_RangeKernel;

};

#endif