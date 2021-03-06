// TrimmedFastMCD.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <iostream>
#include "Utils.h"
#include <numeric>
#include <random>
#include <cmath>
#include <algorithm> 
#include <cstdlib>
#include <fstream>

#include "../Eigen/dense"

/*

namespace csteps
{
	struct BasicCStep
	{
		template <class DistanceCalculation>
		static double newStep(const Eigen::MatrixXd &samples, Algorithms::Solution & localSolution, const Algorithms::Solution &globalSolution)
		{
			const auto n = samples.rows();
			const auto p = samples.cols();
			const auto smd = DistanceCalculation::calculate(samples, globalSolution);
			const double hValue = Algorithms::median(smd)(0);
			const auto subsetSize = static_cast<size_t>(n / 2);
			localSolution.m_inSetMask = Eigen::VectorXi::Zero(n);
			localSolution.m_n = 0;

			Eigen::VectorXi rows(subsetSize);
			auto counter = 0;
			for (auto index = 0; index < n; ++index)
			{
				if (smd(index, 0) <= hValue)
				{
					rows(counter) = index;
					localSolution.m_inSetMask(index) = 1;
					++localSolution.m_n;
					++counter;
				}
			}

			const Eigen::MatrixXd subset = samples(rows, Eigen::placeholders::all).eval();
			localSolution.m_mu = subset.colwise().mean().eval();
			const Eigen::MatrixXd csubset = (subset.rowwise() - localSolution.m_mu).eval();
			localSolution.m_S = (((csubset.transpose() * csubset))).eval();
			localSolution.m_determinant = (localSolution.m_S / (localSolution.m_n - 1)).determinant();
			return hValue / Algorithms::chi2InvSquared05(p);
		}
	};

	struct FastAdaptiveCStep
	{
		template <class DistanceCalculation>
		static double newStep(const Eigen::MatrixXd &samples, Algorithms::Solution & localSolution, const Algorithms::Solution &globalSolution)
		{
			const auto n = samples.rows();
			const auto p = samples.cols();
			const auto smd = DistanceCalculation::calculate(samples, globalSolution);
			const double hValue = Algorithms::median(smd)(0);
			auto wsum = localSolution.m_n;

			for (auto index = 0; index < smd.rows(); ++index)
			{
				const int wasInSet = localSolution.m_inSetMask(index);
				localSolution.m_inSetMask(index) = (smd(index, 0) <= hValue) ? 1 : 0;
				const int diff = localSolution.m_inSetMask(index) - wasInSet;

				if (diff != 0)
				{
					wsum += diff;
					const auto u = (samples.row(index) - localSolution.m_mu).eval();
					localSolution.m_mu += ((diff / (double)wsum) * u).eval();
					const auto v = (samples.row(index) - localSolution.m_mu).eval();
					const auto alpha = u.transpose() * v;
					const auto beta = localSolution.m_SInv * alpha * localSolution.m_SInv;
					const auto gamma = 1 + diff * (v * localSolution.m_SInv * v.transpose())(0);

					localSolution.m_S = localSolution.m_S + diff * alpha;
					localSolution.m_SInv = localSolution.m_SInv - beta / (diff * gamma);
					localSolution.m_determinant = gamma * localSolution.m_determinant;
				}
			}

			localSolution.m_n = wsum;
			return hValue / Algorithms::chi2InvSquared05(p);
		}

	};


	struct AdaptiveCStep
	{
		template <class DistanceCalculation>
		static double newStep(const Eigen::MatrixXd &samples, Algorithms::Solution & localSolution, const Algorithms::Solution &globalSolution)
		{
			const auto n = samples.rows();
			const auto p = samples.cols();
			const auto smd = DistanceCalculation::calculate(samples, globalSolution);
			const double hValue = Algorithms::median(smd)(0);
			auto wsum = localSolution.m_n;

			for (auto index = 0; index < smd.rows(); ++index)
			{
				const int wasInSet = localSolution.m_inSetMask(index);
				localSolution.m_inSetMask(index) = (smd(index, 0) <= hValue) ? 1 : 0;
				const int diff = localSolution.m_inSetMask(index) - wasInSet;

				if (diff != 0)
				{
					wsum += diff;
					const auto d1 = (samples.row(index) - localSolution.m_mu).eval();
					localSolution.m_mu += ((diff / (double)wsum) * d1).eval();
					if (diff > 0)
						localSolution.m_S += (d1.transpose().lazyProduct(samples.row(index) - localSolution.m_mu)).eval();
					else
						localSolution.m_S -= (d1.transpose().lazyProduct(samples.row(index) - localSolution.m_mu)).eval();
				}
			}

			localSolution.m_n = wsum;
			localSolution.m_determinant = (localSolution.m_S / ((double)wsum - 1.0)).determinant();
			return hValue / Algorithms::chi2InvSquared05(p);
		}
	};

}

*/

namespace Solutions 
{
	void AdaptiveCalculation
	(
		const Eigen::MatrixXd &x,
		Eigen::RowVectorXd &mu, Eigen::MatrixXd &sigma, Eigen::MatrixXd &sigmaInv, double &det,
		Eigen::RowVectorXi &insubsetMask, Eigen::RowVectorXi &deltaMask
	)
	{
 		size_t h = std::floor(insubsetMask.size() / 2);
 		double wsum = h;
// 		//	By convention, results should be in the same range
// 		sigma = sigma * (h - 1.0);
// 		sigmaInv = sigmaInv / (h - 1.0);
// 		det = det * h;
		for (auto index = 0; index < deltaMask.size(); ++index)
		{			
			const double diff = deltaMask(index);
			if (diff != 0)
			{
				wsum += diff;
				const auto u = (x.row(index) - mu).eval();
				mu += ((diff / (double)wsum) * u).eval();
				const auto v = (x.row(index) - mu).eval();
				const auto alpha = (u.transpose() * v).eval();
				const auto beta = (sigmaInv * alpha * sigmaInv).eval();
				const auto gamma = 1.0 + diff * (v * sigmaInv * v.transpose())(0);

				sigma = (sigma + diff * alpha).eval();
				sigmaInv = (sigmaInv - beta / (diff * gamma)).eval();
				det = gamma * det;
			}
		}
// 		sigma = sigma / (h - 1.0);
// 		sigmaInv = sigmaInv * (h - 1.0);
// 		det = det / h;
	}


	void NaiveCalculation
	(
		const Eigen::MatrixXd &x, 
		Eigen::RowVectorXd &mu,  Eigen::MatrixXd &sigma, Eigen::MatrixXd &sigmaInv, double &det,
		Eigen::RowVectorXi &insubsetMask, Eigen::RowVectorXi &deltaMask
	)
	{
		const size_t p = mu.size();
		const size_t n = insubsetMask.size();
		const size_t h = std::floor(n / 2);

		//	Construct the hSubset according to the given mask
		Eigen::MatrixXd H = Eigen::MatrixXd(h, p);
		size_t counter = 0;
		for (auto index = 0; index < n; ++index) 
		{
			if (insubsetMask(index) > 0)
			{
				H.row(counter) = x.row(index).eval();
				++counter;
			}
		}

		//	Calculate the required estimations
 		mu = (1.0 / h) * (Eigen::MatrixXd::Ones(1, h) * H).eval();
 		Eigen::MatrixXd HStar = (H - Eigen::MatrixXd::Ones(h, 1) * mu).eval();
 		sigma = ((1.0 / (h - 1.0)) * (HStar.transpose() * HStar).eval());
 		sigmaInv = sigma.inverse().eval();
		det = sigma.determinant();
	}
}

namespace Utils 
{	

	void alterSubset(Eigen::RowVectorXi &mask, Eigen::RowVectorXi &deltaMask, size_t nbrOfElements)
	{
		size_t upcouter = 0;
		size_t downcouter = 0;

		Eigen::RowVectorXi prevMask = mask;

		for (auto index = 0; index < mask.size(); ++index) 
		{
			if ((mask(index) == 0)  && (upcouter < nbrOfElements))
			{
				++upcouter;
				mask(index) = 1;
			}
			else if ((mask(index) == 1) && (downcouter < nbrOfElements))
			{
				++downcouter;
				mask(index) = 0;
			}

			if ((upcouter == downcouter) && (downcouter == nbrOfElements))
			{
				break;
			}
		}

		deltaMask = mask - prevMask;
	}

	Eigen::RowVectorXi generateShuffledIndices(size_t n)
	{
		size_t h = std::floor(n / 2);
		Eigen::RowVectorXi subsetMask = Eigen::RowVectorXi(n);
		Eigen::VectorXi indices = Eigen::VectorXi::LinSpaced(n, 0, n);
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(indices.data(), indices.data() + indices.size(), g);
		for (auto index = 0; index < n; ++index)
		{
			subsetMask(indices(index)) = index < h ? 1 : 0;
		}
		return subsetMask;
	}
}

int main()
{
	Utils::Timer aTimer;

	const auto n = 10000;
	const auto p = 2;
	const auto h = floor(n / 2);
	const size_t incr = h * 0.001;


	//	Generate samples
	Eigen::MatrixXd samples = Eigen::MatrixXd::Random(n, p);

	
	//	Define insetsetMask, delta's
	Eigen::RowVectorXi insubsetMask = Utils::generateShuffledIndices(n);
	Eigen::RowVectorXi deltaMask = insubsetMask - Eigen::RowVectorXi::Zero(n);

	//	Initialize estimations with correct subset value
	Eigen::RowVectorXd mu= Eigen::RowVectorXd::Zero(p);
	Eigen::MatrixXd sigma = Eigen::MatrixXd::Zero(p,p);
	Eigen::MatrixXd sigmaInv = Eigen::MatrixXd::Zero(p, p);
	double det = 0;

	Solutions::NaiveCalculation(samples, mu, sigma, sigmaInv, det, insubsetMask, deltaMask);

	//	Change 'm' elements in the subset 
	Eigen::MatrixXd results = Eigen::MatrixXd::Zero(100, 5);
	size_t counter = 0;
	for (auto nbrOfChangedElements = 1; nbrOfChangedElements <= (h / 10); nbrOfChangedElements += incr) // 20%
	{
		auto tNaive = 0.0;
		for (auto iteration = 0; iteration < 250; ++iteration)
		{
			Utils::alterSubset(insubsetMask, deltaMask, nbrOfChangedElements);
			aTimer.startTimer();
			Solutions::NaiveCalculation(samples, mu, sigma, sigmaInv, det, insubsetMask, deltaMask);
			tNaive += aTimer.stoptimer();
		}

		auto tAdaptive = 0.0;
		for (auto iteration = 0; iteration < 250; ++iteration)
		{
			Utils::alterSubset(insubsetMask, deltaMask, nbrOfChangedElements);
			aTimer.startTimer();
			Solutions::AdaptiveCalculation(samples, mu, sigma, sigmaInv, det, insubsetMask, deltaMask);
			tAdaptive += aTimer.stoptimer();
		}

		results(counter, 0) = nbrOfChangedElements;
		results(counter, 1) = h;
		results(counter, 2) = tNaive;
		results(counter, 3) = tAdaptive;
		results(counter, 4) = (tNaive / tAdaptive);

		++counter;
		std::cout << counter << std::endl;
	}

	std::cout << "#chang 5000  h   tNai   tOpti  speedup" << std::endl;
	std::cout << results << std::endl;

	std::string name = "results.csv";
	const static Eigen::IOFormat CSVFormat(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", "\n");	
	std::ofstream file(name.c_str());
	file << results.format(CSVFormat);
	file.close();
}



