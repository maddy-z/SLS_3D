#ifndef _SLS_BILATERALFILTER_H_
#define _SLS_BILATERALFILTER_H_

#include <vector>

class BilateralFilter
{

public:

	BilateralFilter(double spatialSigma, double rangeSigma, unsigned int kernelSize, unsigned int apprRes);
	virtual ~BilateralFilter();

	bool Filter(const double * src, double * dest, int height, int width, int channel);

private:

	double m_SpatialSigma;
	double m_RangeSigma;

	unsigned int m_KernelSize;
	unsigned int m_ApproxResultion;

	std::vector<double> m_SpatialKernel;
	std::vector<double>	m_RangeKernel;

};

#endif