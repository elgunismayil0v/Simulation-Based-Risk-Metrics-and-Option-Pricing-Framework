#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <numeric>
#include <algorithm>
#include <stdexcept>
using namespace std;

class Heston {
    public:
    vector<double> CIR_sample(
    int NoOfPaths, double kappa, double gamma, double vbar, double s,
     double t, double v_s);
    vector<vector<double> >  GeneratePathsHestonAES(int NoOfPaths, int NoOfSteps, double T,
     double r, double S_0, double kappa,
    double gamma, double rho, double vbar, double v0);
    double CalculateEuropeanCallPrice(const vector<vector<double> >& paths,
    double K, double r, double T);

};

class RiskMetrics{
    public:
    vector<double> CalculateDiscountedExpectedExposureWithStrike(const vector<vector<double> >& paths, const double K, const double r, const double dt);
    vector<double> CalculatePotentialFutureExposure(const vector<vector<double> >& paths,
    double confidence_level, double K);
    double CalculateCVA(const vector<double>& EE, double recovery_rate, double hazard_rate, double T, int NoOfSteps);
};

vector<double> Heston::CIR_sample(int NoOfPaths, double kappa, double gamma, double vbar, double s, double t, double v_s)
{
    // Parameters for the non-central chi-squared distribution
    double delta = 4.0 * kappa * vbar / (gamma * gamma);
    double exp_factor = exp(-kappa * (t - s));
    double c = (gamma * gamma * (1.0 - exp_factor)) / (4.0 * kappa);
    double lambda = (4.0 * kappa * v_s * exp_factor) / (gamma * gamma * (1.0 - exp_factor));

    // Random number generator setup
    random_device rd;
    mt19937 generator(rd());
    normal_distribution<> normal_dist(0.0, 1.0);

    vector<double> samples(NoOfPaths);

    // Generate samples using the non-central chi-squared approximation
    for (int i = 0; i < NoOfPaths; ++i) {
        // Generate central chi-squared part
        double central_sum = 0.0;
        for (int j = 0; j < static_cast<int>(delta); ++j) {
            double z = normal_dist(generator);
            central_sum += z * z;
        }

        // Add non-centrality parameter
        double non_central_part = lambda;
        samples[i] = c * (central_sum + non_central_part);

        // Ensure non-negativity
        samples[i] = max(samples[i], 0.0);
    }

    return samples;
}

vector<vector<double> > Heston ::GeneratePathsHestonAES( int NoOfPaths, int NoOfSteps, double T, double r, double S_0, double kappa,
    double gamma, double rho, double vbar, double v0) {
        // Random number generation
    random_device rd;
    mt19937 gen(rd());
    normal_distribution<> normal_dist(0.0, 1.0);

    // Initialize matrices and vectors
    vector<vector<double> > Z1(NoOfPaths, vector<double>(NoOfSteps));
    vector<vector<double> > W1(NoOfPaths, vector<double>(NoOfSteps + 1, 0.0));
    vector<vector<double> > V(NoOfPaths, vector<double>(NoOfSteps + 1, 0.0));
    vector<vector<double> > X(NoOfPaths, vector<double>(NoOfSteps + 1, 0.0));
    vector<double> time(NoOfSteps + 1, 0.0);

    double dt = T / static_cast<double>(NoOfSteps);

    // Initial conditions
    for (int i = 0; i < NoOfPaths; ++i) {
        V[i][0] = v0;
        X[i][0] = log(S_0);
    };

    // Generate paths
    for (int step = 0; step < NoOfSteps; ++step) {
        // Generate random samples
        for (int i = 0; i < NoOfPaths; ++i) {
            Z1[i][step] = normal_dist(gen);
        };

        // Normalize Z1 to ensure mean = 0, variance = 1
        if (NoOfPaths > 1) {
            vector<double> column(NoOfPaths);
            for (int i = 0; i < NoOfPaths; ++i) column[i] = Z1[i][step];
            // normalize(column); // Optional, if normalization logic is defined
            for (int i = 0; i < NoOfPaths; ++i) Z1[i][step] = column[i];
        }

        // Update Wiener process
        for (int i = 0; i < NoOfPaths; ++i) {
            W1[i][step + 1] = W1[i][step] + sqrt(dt) * Z1[i][step];
        }

        // Generate next variance values using the CIR model
        vector<double> next_v = CIR_sample(NoOfPaths, kappa, gamma, vbar, 0.0, dt, V[0][step]);

        // Update variance for each path
        for (int i = 0; i < NoOfPaths; ++i) {
            V[i][step + 1] = next_v[i]; // Assign each path's next variance
        }

        // Update log price process
        for (int i = 0; i < NoOfPaths; ++i) {
            double k0 = (r - rho / gamma * kappa * vbar) * dt;
            double k1 = (rho * kappa / gamma - 0.5) * dt - rho / gamma;
            double k2 = rho / gamma;
            double dW1 = W1[i][step + 1] - W1[i][step];
            X[i][step + 1] = X[i][step] + k0 + k1 * V[i][step] +
                             k2 * V[i][step + 1] +
                             sqrt((1.0 - rho * rho) * V[i][step]) * dW1;
        }

        // Update time
        time[step + 1] = time[step] + dt;
    }

    // Compute exponent and store results
    vector<vector<double> > S(NoOfPaths, vector<double>(NoOfSteps + 1));
    for (int i = 0; i < NoOfPaths; ++i) {
        for (int step = 0; step <= NoOfSteps; ++step) {
            S[i][step] = exp(X[i][step]);
        }
    }

    return S; // Returns the simulated paths

};

double Heston:: CalculateEuropeanCallPrice(
    const vector<vector<double> >& paths,
    double K, double r, double T) {
    int NoOfPaths = paths.size();
    vector<double> payoffs(NoOfPaths);

    // Calculate payoff for each path
    for (int i = 0; i < NoOfPaths; ++i) {
        double S_T = paths[i].back(); // Final price at maturity
        payoffs[i] = max(S_T - K, 0.0);
    }

    // Average payoff and discount
    double avg_payoff = accumulate(payoffs.begin(), payoffs.end(), 0.0) / NoOfPaths;
    return exp(-r * T) * avg_payoff;
}

vector<double> RiskMetrics::CalculateDiscountedExpectedExposureWithStrike(const vector<vector<double> >& paths, const double K, const double r, const double dt) {
    if (paths.empty() || paths[0].empty()) {
        throw std::invalid_argument("Input paths must be non-empty.");
    }

    int NoOfSteps = paths[0].size();
    int NoOfPaths = paths.size();
    vector<double> DiscountedEE(NoOfSteps, 0.0);

    // Calculate discounted EE
    for (int step = 0; step < NoOfSteps; ++step) {
        double sum_of_positive_exposures = 0.0;
        for (int i = 0; i < NoOfPaths; ++i) {
            sum_of_positive_exposures += max(paths[i][step] - K, 0.0);
        }
        double discount_factor = exp(-r * step * dt); // Discount factor
        DiscountedEE[step] = (sum_of_positive_exposures / NoOfPaths) * discount_factor;
    }

    return DiscountedEE;
}

vector<double> RiskMetrics::CalculatePotentialFutureExposure(
    const vector<vector<double> >& paths, double confidence_level, double K) {
    int NoOfSteps = paths[0].size(); // Number of time steps
    int NoOfPaths = paths.size();   // Number of paths
    vector<double> PFE(NoOfSteps, 0.0);

    // Calculate PFE for each time step
    for (int step = 0; step < NoOfSteps; ++step) {
        vector<double> exposures(NoOfPaths);

        // Collect exposures at this time step
        for (int i = 0; i < NoOfPaths; ++i) {
            exposures[i] = max(paths[i][step] - K, 0.0);
        }

        // Sort exposures to compute quantile
        sort(exposures.begin(), exposures.end());
        int index = static_cast<int>(confidence_level * NoOfPaths) - 1;
        PFE[step] = exposures[index];
    }

    return PFE;
}

double RiskMetrics ::CalculateCVA(const vector<double>& EE, double recovery_rate, double hazard_rate, double T, int NoOfSteps) {
    double dt = T / NoOfSteps;
    double CVA = 0.0;

    for (int step = 0; step < NoOfSteps; ++step) {
        double t = step * dt;
        double PD_t = 1 - exp(-hazard_rate * t);
        CVA += (1 - recovery_rate) * EE[step] * PD_t * dt;
    }

    return CVA;
}




