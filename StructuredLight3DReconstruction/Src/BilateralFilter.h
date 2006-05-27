#ifndef _SLS_BILATERALFILTER_H_
#define _SLS_BILATERALFILTER_H_

#include <vector>

class BilateralFilter

{

public:

	// 
	// Constructor & Destructor
	// 

	BilateralFilter(double spatialSigma, double rangeSigma, int kernelSize, int apprRes = 256);
	virtual ~BilateralFilter();

	// 
	// Filtering 2-Dimension Image
	// 

	bool Filter(const double * src, double * dest, int height, int width, int channel);

	void PrintFilterInfo() const
	{
		printf("Bilateral Filter Information:\n\n");
		
		printf("Spatial Sigma = %f\n", m_SpatialSigma);
		printf("Range Sigma= %f\n", m_RangeSigma);
		printf("Kernel Size = %d\n", m_KernelSize);
		printf("Approximate Resolution = %d\n", m_ApproxResolution);
	}

private:

	double m_SpatialSigma;
	double m_RangeSigma;

	int m_Radius;
	int m_KernelSize;
	int m_ApproxResolution;

	std::vector<double> m_SpatialKernel;
	std::vector<int> m_SpatialOffset;

	std::vector<double>	m_RangeKernel;
};

#endif