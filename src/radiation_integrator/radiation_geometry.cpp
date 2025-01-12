// Blacklight radiation integrator - geodesic manipulation and geometry

// C++ headers
#include <cmath>  // acos, atan, atan2, cos, hypot, sin, sqrt

// Blacklight headers
#include "radiation_integrator.hpp"
#include "../blacklight.hpp"         // Math, enums
#include "../utils/array.hpp"        // Array

//--------------------------------------------------------------------------------------------------

// Function for calculating radial coordinate given location in coordinates used for geodesics
// Inputs:
//   x, y, z: Cartesian Kerr-Schild coordinates
// Outputs:
//   returned value: radial spherical Kerr-Schild coordinate
double RadiationIntegrator::RadialGeodesicCoordinate(double x, double y, double z) const
{
  double a2 = bh_a * bh_a;
  double rr2 = x * x + y * y + z * z;
  double r2 = 0.5 * (rr2 - a2 + std::hypot(rr2 - a2, 2.0 * bh_a * z));
  double r = std::sqrt(r2);
  return r;
}

//--------------------------------------------------------------------------------------------------

// Function for converting from Cartesian Kerr-Schild to simulation coordinates
// Inputs:
//   *p_x1, *p_x2, *p_x3: Cartesian Kerr-Schild coordinates
// Outputs:
//   *p_x1, *p_x2, *p_x3: simulation coordinates
void RadiationIntegrator::ConvertFromCKS(double *p_x1, double *p_x2, double *p_x3) const
{
  if (simulation_coord == Coordinates::sks)
  {
    double x = *p_x1;
    double y = *p_x2;
    double z = *p_x3;
    double a2 = bh_a * bh_a;
    double rr2 = x * x + y * y + z * z;
    double r2 = 0.5 * (rr2 - a2 + std::hypot(rr2 - a2, 2.0 * bh_a * z));
    double r = std::sqrt(r2);
    double th = std::acos(z / r);
    double ph = std::atan2(y, x) - std::atan(bh_a / r);
    ph += ph < 0.0 ? 2.0 * Math::pi : 0.0;
    ph -= ph >= 2.0 * Math::pi ? 2.0 * Math::pi : 0.0;
    *p_x1 = r;
    *p_x2 = th;
    *p_x3 = ph;
  }
  return;
}

//--------------------------------------------------------------------------------------------------

// Function for calculating Jacobian of transformation between geodesic and simulation coordinates
// Inputs:
//   x, y, z: Cartesian Kerr-Schild coordinates
// Outputs:
//   jacobian: components set
// Notes:
//   Assumes jacobian is allocated to be 4*4.
//   Jacobian contains dx_geodesic^mu/dx_simulation^nu with indices (mu, nu).
void RadiationIntegrator::CoordinateJacobian(double x, double y, double z, double jacobian[4][4])
    const
{
  // Calculate Jacobian for spherical Kerr-Schild simulation
  if (simulation_coord == Coordinates::sks)
  {
    // Calculate spherical position
    double a2 = bh_a * bh_a;
    double rr2 = x * x + y * y + z * z;
    double r2 = 0.5 * (rr2 - a2 + std::hypot(rr2 - a2, 2.0 * bh_a * z));
    double r = std::sqrt(r2);
    double cth = z / r;
    double sth = std::sqrt(1.0 - cth * cth);
    double ph = std::atan2(y, x) - std::atan(bh_a / r);
    double sph = std::sin(ph);
    double cph = std::cos(ph);

    // Calculate Jacobian of transformation
    jacobian[0][0] = 1.0;
    jacobian[0][1] = 0.0;
    jacobian[0][2] = 0.0;
    jacobian[0][3] = 0.0;
    jacobian[1][0] = 0.0;
    jacobian[1][1] = sth * cph;
    jacobian[1][2] = cth * (r * cph - bh_a * sph);
    jacobian[1][3] = sth * (-r * sph - bh_a * cph);
    jacobian[2][0] = 0.0;
    jacobian[2][1] = sth * sph;
    jacobian[2][2] = cth * (r * sph + bh_a * cph);
    jacobian[2][3] = sth * (r * cph - bh_a * sph);
    jacobian[3][0] = 0.0;
    jacobian[3][1] = cth;
    jacobian[3][2] = -r * sth;
    jacobian[3][3] = 0.0;
  }

  // Calculate Jacobian for Cartesian Kerr-Schild simulation
  else if (simulation_coord == Coordinates::cks)
  {
    jacobian[0][0] = 1.0;
    jacobian[0][1] = 0.0;
    jacobian[0][2] = 0.0;
    jacobian[0][3] = 0.0;
    jacobian[1][0] = 0.0;
    jacobian[1][1] = 1.0;
    jacobian[1][2] = 0.0;
    jacobian[1][3] = 0.0;
    jacobian[2][0] = 0.0;
    jacobian[2][1] = 0.0;
    jacobian[2][2] = 1.0;
    jacobian[2][3] = 0.0;
    jacobian[3][0] = 0.0;
    jacobian[3][1] = 0.0;
    jacobian[3][2] = 0.0;
    jacobian[3][3] = 1.0;
  }
  return;
}

//--------------------------------------------------------------------------------------------------

// Function for calculating covariant metric components in geodesic coordinates
// Inputs:
//   x, y, z: Cartesian Kerr-Schild coordinates
// Outputs:
//   gcov: components set
// Notes:
//   Assumes gcov is allocated to be 4*4.
//   Assumes Minkowski coordinates if ray_flat == true.
void RadiationIntegrator::CovariantGeodesicMetric(double x, double y, double z, double gcov[4][4])
    const
{
  // Handle flat case
  if (ray_flat)
  {
    gcov[0][0] = -1.0;
    gcov[0][1] = 0.0;
    gcov[0][2] = 0.0;
    gcov[0][3] = 0.0;
    gcov[1][0] = 0.0;
    gcov[1][1] = 1.0;
    gcov[1][2] = 0.0;
    gcov[1][3] = 0.0;
    gcov[2][0] = 0.0;
    gcov[2][1] = 0.0;
    gcov[2][2] = 1.0;
    gcov[2][3] = 0.0;
    gcov[3][0] = 0.0;
    gcov[3][1] = 0.0;
    gcov[3][2] = 0.0;
    gcov[3][3] = 1.0;
    return;
  }

  // Calculate useful quantities
  double a2 = bh_a * bh_a;
  double rr2 = x * x + y * y + z * z;
  double r2 = 0.5 * (rr2 - a2 + std::hypot(rr2 - a2, 2.0 * bh_a * z));
  double r = std::sqrt(r2);
  double f = 2.0 * bh_m * r2 * r / (r2 * r2 + a2 * z * z);

  // Calculate null vector
  double l_0 = 1.0;
  double l_1 = (r * x + bh_a * y) / (r2 + a2);
  double l_2 = (r * y - bh_a * x) / (r2 + a2);
  double l_3 = z / r;

  // Calculate metric components
  gcov[0][0] = f * l_0 * l_0 - 1.0;
  gcov[0][1] = f * l_0 * l_1;
  gcov[0][2] = f * l_0 * l_2;
  gcov[0][3] = f * l_0 * l_3;
  gcov[1][0] = f * l_1 * l_0;
  gcov[1][1] = f * l_1 * l_1 + 1.0;
  gcov[1][2] = f * l_1 * l_2;
  gcov[1][3] = f * l_1 * l_3;
  gcov[2][0] = f * l_2 * l_0;
  gcov[2][1] = f * l_2 * l_1;
  gcov[2][2] = f * l_2 * l_2 + 1.0;
  gcov[2][3] = f * l_2 * l_3;
  gcov[3][0] = f * l_3 * l_0;
  gcov[3][1] = f * l_3 * l_1;
  gcov[3][2] = f * l_3 * l_2;
  gcov[3][3] = f * l_3 * l_3 + 1.0;
  return;
}

//--------------------------------------------------------------------------------------------------

// Function for calculating contravariant metric components in geodesic coordinates
// Inputs:
//   x, y, z: Cartesian Kerr-Schild coordinates
// Outputs:
//   gcon: components set
// Notes:
//   Assumes gcon is allocated to be 4*4.
//   Assumes Minkowski coordinates if ray_flat == true.
void RadiationIntegrator::ContravariantGeodesicMetric(double x, double y, double z,
    double gcon[4][4]) const
{
  // Handle flat case
  if (ray_flat)
  {
    gcon[0][0] = -1.0;
    gcon[0][1] = 0.0;
    gcon[0][2] = 0.0;
    gcon[0][3] = 0.0;
    gcon[1][0] = 0.0;
    gcon[1][1] = 1.0;
    gcon[1][2] = 0.0;
    gcon[1][3] = 0.0;
    gcon[2][0] = 0.0;
    gcon[2][1] = 0.0;
    gcon[2][2] = 1.0;
    gcon[2][3] = 0.0;
    gcon[3][0] = 0.0;
    gcon[3][1] = 0.0;
    gcon[3][2] = 0.0;
    gcon[3][3] = 1.0;
    return;
  }

  // Calculate useful quantities
  double a2 = bh_a * bh_a;
  double rr2 = x * x + y * y + z * z;
  double r2 = 0.5 * (rr2 - a2 + std::hypot(rr2 - a2, 2.0 * bh_a * z));
  double r = std::sqrt(r2);
  double f = 2.0 * bh_m * r2 * r / (r2 * r2 + a2 * z * z);

  // Calculate null vector
  double l0 = -1.0;
  double l1 = (r * x + bh_a * y) / (r2 + a2);
  double l2 = (r * y - bh_a * x) / (r2 + a2);
  double l3 = z / r;

  // Calculate metric components
  gcon[0][0] = -f * l0 * l0 - 1.0;
  gcon[0][1] = -f * l0 * l1;
  gcon[0][2] = -f * l0 * l2;
  gcon[0][3] = -f * l0 * l3;
  gcon[1][0] = -f * l1 * l0;
  gcon[1][1] = -f * l1 * l1 + 1.0;
  gcon[1][2] = -f * l1 * l2;
  gcon[1][3] = -f * l1 * l3;
  gcon[2][0] = -f * l2 * l0;
  gcon[2][1] = -f * l2 * l1;
  gcon[2][2] = -f * l2 * l2 + 1.0;
  gcon[2][3] = -f * l2 * l3;
  gcon[3][0] = -f * l3 * l0;
  gcon[3][1] = -f * l3 * l1;
  gcon[3][2] = -f * l3 * l2;
  gcon[3][3] = -f * l3 * l3 + 1.0;
  return;
}

//--------------------------------------------------------------------------------------------------

// Function for calculating Christoffel connection components in geodesic coordinates
// Inputs:
//   x, y, z: Cartesian Kerr-Schild coordinates
// Outputs:
//   connection: components set
// Notes:
//   Assumes connection is allocated to be 4*4*4.
//   Assumes Minkowski coordinates if ray_flat == true.
void RadiationIntegrator::GeodesicConnection(double x, double y, double z,
    double connection[4][4][4]) const
{
  // Handle flat case
  if (ray_flat)
  {
    for (int mu = 0; mu < 4; mu++)
      for (int alpha = 0; alpha < 4; alpha++)
        for (int beta = 0; beta < 4; beta++)
          connection[mu][alpha][beta] = 0.0;
    return;
  }

  // Calculate useful quantities
  double a2 = bh_a * bh_a;
  double rr2 = x * x + y * y + z * z;
  double r2 = 0.5 * (rr2 - a2 + std::hypot(rr2 - a2, 2.0 * bh_a * z));
  double r = std::sqrt(r2);
  double f = 2.0 * bh_m * r2 * r / (r2 * r2 + a2 * z * z);

  // Calculate null vector
  double l0 = -1.0;
  double l1 = (r * x + bh_a * y) / (r2 + a2);
  double l2 = (r * y - bh_a * x) / (r2 + a2);
  double l3 = z / r;

  // Calculate metric components
  double gcon[4][4];
  gcon[0][0] = -f * l0 * l0 - 1.0;
  gcon[0][1] = -f * l0 * l1;
  gcon[0][2] = -f * l0 * l2;
  gcon[0][3] = -f * l0 * l3;
  gcon[1][0] = -f * l1 * l0;
  gcon[1][1] = -f * l1 * l1 + 1.0;
  gcon[1][2] = -f * l1 * l2;
  gcon[1][3] = -f * l1 * l3;
  gcon[2][0] = -f * l2 * l0;
  gcon[2][1] = -f * l2 * l1;
  gcon[2][2] = -f * l2 * l2 + 1.0;
  gcon[2][3] = -f * l2 * l3;
  gcon[3][0] = -f * l3 * l0;
  gcon[3][1] = -f * l3 * l1;
  gcon[3][2] = -f * l3 * l2;
  gcon[3][3] = -f * l3 * l3 + 1.0;

  // Calculate scalar derivatives
  double dr_dx = r * x / (2.0 * r2 - rr2 + a2);
  double dr_dy = r * y / (2.0 * r2 - rr2 + a2);
  double dr_dz = (r * z + a2 * z / r) / (2.0 * r2 - rr2 + a2);
  double df_dx = -(r2 * r2 - 3.0 * a2 * z * z) * dr_dx / (r * (r2 * r2 + a2 * z * z)) * f;
  double df_dy = -(r2 * r2 - 3.0 * a2 * z * z) * dr_dy / (r * (r2 * r2 + a2 * z * z)) * f;
  double df_dz = -((r2 * r2 - 3.0 * a2 * z * z) * dr_dz + 2.0 * a2 * r * z)
      / (r * (r2 * r2 + a2 * z * z)) * f;

  // Calculate vector derivatives
  double dl0_dx = 0.0;
  double dl0_dy = 0.0;
  double dl0_dz = 0.0;
  double dl1_dx = ((x - 2.0 * r * l1) * dr_dx + r) / (r2 + a2);
  double dl1_dy = ((x - 2.0 * r * l1) * dr_dy + bh_a) / (r2 + a2);
  double dl1_dz = (x - 2.0 * r * l1) * dr_dz / (r2 + a2);
  double dl2_dx = ((y - 2.0 * r * l2) * dr_dx - bh_a) / (r2 + a2);
  double dl2_dy = ((y - 2.0 * r * l2) * dr_dy + r) / (r2 + a2);
  double dl2_dz = (y - 2.0 * r * l2) * dr_dz / (r2 + a2);
  double dl3_dx = -z / r2 * dr_dx;
  double dl3_dy = -z / r2 * dr_dy;
  double dl3_dz = -z / r2 * dr_dz + 1.0 / r;

  // Prepare array for covariant metric component derivatives
  double dgcov[4][4][4] = {};

  // Calculate covariant metric component x-derivatives
  dgcov[1][0][0] = +(df_dx * l0 * l0 + f * dl0_dx * l0 + f * l0 * dl0_dx);
  dgcov[1][0][1] = -(df_dx * l0 * l1 + f * dl0_dx * l1 + f * l0 * dl1_dx);
  dgcov[1][0][2] = -(df_dx * l0 * l2 + f * dl0_dx * l2 + f * l0 * dl2_dx);
  dgcov[1][0][3] = -(df_dx * l0 * l3 + f * dl0_dx * l3 + f * l0 * dl3_dx);
  dgcov[1][1][0] = -(df_dx * l1 * l0 + f * dl1_dx * l0 + f * l1 * dl0_dx);
  dgcov[1][1][1] = +(df_dx * l1 * l1 + f * dl1_dx * l1 + f * l1 * dl1_dx);
  dgcov[1][1][2] = +(df_dx * l1 * l2 + f * dl1_dx * l2 + f * l1 * dl2_dx);
  dgcov[1][1][3] = +(df_dx * l1 * l3 + f * dl1_dx * l3 + f * l1 * dl3_dx);
  dgcov[1][2][0] = -(df_dx * l2 * l0 + f * dl2_dx * l0 + f * l2 * dl0_dx);
  dgcov[1][2][1] = +(df_dx * l2 * l1 + f * dl2_dx * l1 + f * l2 * dl1_dx);
  dgcov[1][2][2] = +(df_dx * l2 * l2 + f * dl2_dx * l2 + f * l2 * dl2_dx);
  dgcov[1][2][3] = +(df_dx * l2 * l3 + f * dl2_dx * l3 + f * l2 * dl3_dx);
  dgcov[1][3][0] = -(df_dx * l3 * l0 + f * dl3_dx * l0 + f * l3 * dl0_dx);
  dgcov[1][3][1] = +(df_dx * l3 * l1 + f * dl3_dx * l1 + f * l3 * dl1_dx);
  dgcov[1][3][2] = +(df_dx * l3 * l2 + f * dl3_dx * l2 + f * l3 * dl2_dx);
  dgcov[1][3][3] = +(df_dx * l3 * l3 + f * dl3_dx * l3 + f * l3 * dl3_dx);

  // Calculate covariant metric component y-derivatives
  dgcov[2][0][0] = +(df_dy * l0 * l0 + f * dl0_dy * l0 + f * l0 * dl0_dy);
  dgcov[2][0][1] = -(df_dy * l0 * l1 + f * dl0_dy * l1 + f * l0 * dl1_dy);
  dgcov[2][0][2] = -(df_dy * l0 * l2 + f * dl0_dy * l2 + f * l0 * dl2_dy);
  dgcov[2][0][3] = -(df_dy * l0 * l3 + f * dl0_dy * l3 + f * l0 * dl3_dy);
  dgcov[2][1][0] = -(df_dy * l1 * l0 + f * dl1_dy * l0 + f * l1 * dl0_dy);
  dgcov[2][1][1] = +(df_dy * l1 * l1 + f * dl1_dy * l1 + f * l1 * dl1_dy);
  dgcov[2][1][2] = +(df_dy * l1 * l2 + f * dl1_dy * l2 + f * l1 * dl2_dy);
  dgcov[2][1][3] = +(df_dy * l1 * l3 + f * dl1_dy * l3 + f * l1 * dl3_dy);
  dgcov[2][2][0] = -(df_dy * l2 * l0 + f * dl2_dy * l0 + f * l2 * dl0_dy);
  dgcov[2][2][1] = +(df_dy * l2 * l1 + f * dl2_dy * l1 + f * l2 * dl1_dy);
  dgcov[2][2][2] = +(df_dy * l2 * l2 + f * dl2_dy * l2 + f * l2 * dl2_dy);
  dgcov[2][2][3] = +(df_dy * l2 * l3 + f * dl2_dy * l3 + f * l2 * dl3_dy);
  dgcov[2][3][0] = -(df_dy * l3 * l0 + f * dl3_dy * l0 + f * l3 * dl0_dy);
  dgcov[2][3][1] = +(df_dy * l3 * l1 + f * dl3_dy * l1 + f * l3 * dl1_dy);
  dgcov[2][3][2] = +(df_dy * l3 * l2 + f * dl3_dy * l2 + f * l3 * dl2_dy);
  dgcov[2][3][3] = +(df_dy * l3 * l3 + f * dl3_dy * l3 + f * l3 * dl3_dy);

  // Calculate covariant metric component z-derivatives
  dgcov[3][0][0] = +(df_dz * l0 * l0 + f * dl0_dz * l0 + f * l0 * dl0_dz);
  dgcov[3][0][1] = -(df_dz * l0 * l1 + f * dl0_dz * l1 + f * l0 * dl1_dz);
  dgcov[3][0][2] = -(df_dz * l0 * l2 + f * dl0_dz * l2 + f * l0 * dl2_dz);
  dgcov[3][0][3] = -(df_dz * l0 * l3 + f * dl0_dz * l3 + f * l0 * dl3_dz);
  dgcov[3][1][0] = -(df_dz * l1 * l0 + f * dl1_dz * l0 + f * l1 * dl0_dz);
  dgcov[3][1][1] = +(df_dz * l1 * l1 + f * dl1_dz * l1 + f * l1 * dl1_dz);
  dgcov[3][1][2] = +(df_dz * l1 * l2 + f * dl1_dz * l2 + f * l1 * dl2_dz);
  dgcov[3][1][3] = +(df_dz * l1 * l3 + f * dl1_dz * l3 + f * l1 * dl3_dz);
  dgcov[3][2][0] = -(df_dz * l2 * l0 + f * dl2_dz * l0 + f * l2 * dl0_dz);
  dgcov[3][2][1] = +(df_dz * l2 * l1 + f * dl2_dz * l1 + f * l2 * dl1_dz);
  dgcov[3][2][2] = +(df_dz * l2 * l2 + f * dl2_dz * l2 + f * l2 * dl2_dz);
  dgcov[3][2][3] = +(df_dz * l2 * l3 + f * dl2_dz * l3 + f * l2 * dl3_dz);
  dgcov[3][3][0] = -(df_dz * l3 * l0 + f * dl3_dz * l0 + f * l3 * dl0_dz);
  dgcov[3][3][1] = +(df_dz * l3 * l1 + f * dl3_dz * l1 + f * l3 * dl1_dz);
  dgcov[3][3][2] = +(df_dz * l3 * l2 + f * dl3_dz * l2 + f * l3 * dl2_dz);
  dgcov[3][3][3] = +(df_dz * l3 * l3 + f * dl3_dz * l3 + f * l3 * dl3_dz);

  // Calculate connection coefficients
  for (int mu = 0; mu < 4; mu++)
    for (int alpha = 0; alpha < 4; alpha++)
      for (int beta = 0; beta < 4; beta++)
      {
        connection[mu][alpha][beta] = 0.0;
        for (int nu = 0; nu < 4; nu++)
          connection[mu][alpha][beta] += 0.5 * gcon[mu][nu]
              * (dgcov[alpha][beta][nu] + dgcov[beta][alpha][nu] - dgcov[nu][alpha][beta]);
      }
  return;
}

//--------------------------------------------------------------------------------------------------

// Function for calculating covariant metric components in simulation coordinates
// Inputs:
//   x, y, z: Cartesian Kerr-Schild coordinates
// Outputs:
//   gcov: components set
// Notes:
//   Assumes gcov is allocated to be 4*4.
void RadiationIntegrator::CovariantSimulationMetric(double x, double y, double z, double gcov[4][4])
    const
{
  // Calculate spherical Kerr-Schild metric
  if (simulation_coord == Coordinates::sks)
  {
    // Calculate useful quantities
    double a2 = bh_a * bh_a;
    double rr2 = x * x + y * y + z * z;
    double r2 = 0.5 * (rr2 - a2 + std::hypot(rr2 - a2, 2.0 * bh_a * z));
    double r = std::sqrt(r2);
    double cth = z / r;
    double cth2 = cth * cth;
    double sth2 = 1.0 - cth2;
    double sigma = r2 + a2 * cth2;

    // Calculate metric components
    gcov[0][0] = -(1.0 - 2.0 * bh_m * r / sigma);
    gcov[0][1] = 2.0 * bh_m * r / sigma;
    gcov[0][2] = 0.0;
    gcov[0][3] = -2.0 * bh_m * bh_a * r * sth2 / sigma;
    gcov[1][0] = 2.0 * bh_m * r / sigma;
    gcov[1][1] = 1.0 + 2.0 * bh_m * r / sigma;
    gcov[1][2] = 0.0;
    gcov[1][3] = -(1.0 + 2.0 * bh_m * r / sigma) * bh_a * sth2;
    gcov[2][0] = 0.0;
    gcov[2][1] = 0.0;
    gcov[2][2] = sigma;
    gcov[2][3] = 0.0;
    gcov[3][0] = -2.0 * bh_m * bh_a * r * sth2 / sigma;
    gcov[3][1] = -(1.0 + 2.0 * bh_m * r / sigma) * bh_a * sth2;
    gcov[3][2] = 0.0;
    gcov[3][3] = (r2 + a2 + 2.0 * bh_m * a2 * r * sth2 / sigma) * sth2;
  }

  // Calculate Cartesian Kerr-Schild metric
  else if (simulation_coord == Coordinates::cks)
  {
    // Calculate useful quantities
    double a2 = bh_a * bh_a;
    double rr2 = x * x + y * y + z * z;
    double r2 = 0.5 * (rr2 - a2 + std::hypot(rr2 - a2, 2.0 * bh_a * z));
    double r = std::sqrt(r2);
    double f = 2.0 * bh_m * r2 * r / (r2 * r2 + a2 * z * z);

    // Calculate null vector
    double l_0 = 1.0;
    double l_1 = (r * x + bh_a * y) / (r2 + a2);
    double l_2 = (r * y - bh_a * x) / (r2 + a2);
    double l_3 = z / r;

    // Calculate metric components
    gcov[0][0] = f * l_0 * l_0 - 1.0;
    gcov[0][1] = f * l_0 * l_1;
    gcov[0][2] = f * l_0 * l_2;
    gcov[0][3] = f * l_0 * l_3;
    gcov[1][0] = f * l_1 * l_0;
    gcov[1][1] = f * l_1 * l_1 + 1.0;
    gcov[1][2] = f * l_1 * l_2;
    gcov[1][3] = f * l_1 * l_3;
    gcov[2][0] = f * l_2 * l_0;
    gcov[2][1] = f * l_2 * l_1;
    gcov[2][2] = f * l_2 * l_2 + 1.0;
    gcov[2][3] = f * l_2 * l_3;
    gcov[3][0] = f * l_3 * l_0;
    gcov[3][1] = f * l_3 * l_1;
    gcov[3][2] = f * l_3 * l_2;
    gcov[3][3] = f * l_3 * l_3 + 1.0;
  }
  return;
}

//--------------------------------------------------------------------------------------------------

// Function for calculating contravariant metric components in simulation coordinates
// Inputs:
//   x, y, z: Cartesian Kerr-Schild coordinates
// Outputs:
//   gcon: components set
// Notes:
//   Assumes gcon is allocated to be 4*4.
void RadiationIntegrator::ContravariantSimulationMetric(double x, double y, double z,
    double gcon[4][4]) const
{
  // Calculate spherical Kerr-Schild metric
  if (simulation_coord == Coordinates::sks)
  {
    // Calculate useful quantities
    double a2 = bh_a * bh_a;
    double rr2 = x * x + y * y + z * z;
    double r2 = 0.5 * (rr2 - a2 + std::hypot(rr2 - a2, 2.0 * bh_a * z));
    double r = std::sqrt(r2);
    double cth = z / r;
    double cth2 = cth * cth;
    double sth2 = 1.0 - cth2;
    double delta = r2 - 2.0 * bh_m * r + a2;
    double sigma = r2 + a2 * cth2;

    // Calculate metric components
    gcon[0][0] = -(1.0 + 2.0 * bh_m * r / sigma);
    gcon[0][1] = 2.0 * bh_m * r / sigma;
    gcon[0][2] = 0.0;
    gcon[0][3] = 0.0;
    gcon[1][0] = 2.0 * bh_m * r / sigma;
    gcon[1][1] = delta / sigma;
    gcon[1][2] = 0.0;
    gcon[1][3] = bh_a / sigma;
    gcon[2][0] = 0.0;
    gcon[2][1] = 0.0;
    gcon[2][2] = 1.0 / sigma;
    gcon[2][3] = 0.0;
    gcon[3][0] = 0.0;
    gcon[3][1] = bh_a / sigma;
    gcon[3][2] = 0.0;
    gcon[3][3] = 1.0 / (sigma * sth2);
  }

  // Calculate Cartesian Kerr-Schild metric
  else if (simulation_coord == Coordinates::cks)
  {
    // Calculate useful quantities
    double a2 = bh_a * bh_a;
    double rr2 = x * x + y * y + z * z;
    double r2 = 0.5 * (rr2 - a2 + std::hypot(rr2 - a2, 2.0 * bh_a * z));
    double r = std::sqrt(r2);
    double f = 2.0 * bh_m * r2 * r / (r2 * r2 + a2 * z * z);

    // Calculate null vector
    double l0 = -1.0;
    double l1 = (r * x + bh_a * y) / (r2 + a2);
    double l2 = (r * y - bh_a * x) / (r2 + a2);
    double l3 = z / r;

    // Calculate metric components
    gcon[0][0] = -f * l0 * l0 - 1.0;
    gcon[0][1] = -f * l0 * l1;
    gcon[0][2] = -f * l0 * l2;
    gcon[0][3] = -f * l0 * l3;
    gcon[1][0] = -f * l1 * l0;
    gcon[1][1] = -f * l1 * l1 + 1.0;
    gcon[1][2] = -f * l1 * l2;
    gcon[1][3] = -f * l1 * l3;
    gcon[2][0] = -f * l2 * l0;
    gcon[2][1] = -f * l2 * l1;
    gcon[2][2] = -f * l2 * l2 + 1.0;
    gcon[2][3] = -f * l2 * l3;
    gcon[3][0] = -f * l3 * l0;
    gcon[3][1] = -f * l3 * l1;
    gcon[3][2] = -f * l3 * l2;
    gcon[3][3] = -f * l3 * l3 + 1.0;
  }
  return;
}

//--------------------------------------------------------------------------------------------------

// Function for calculating orthonormal tetrad components in geodesic coordinates
// Inputs:
//   ucon: contravariant components of timelike velocity
//   ucov: covariant components of timelike velocity
//   kcon: contravariant components of null momentum
//   kcov: covariant components of null momentum
//   up_con: contravariant components of spacelike up direction (usually magnetic field)
//   gcov: covariant metric components
//   gcon: contravariant metric components
// Outputs:
//   tetrad: components set
// Notes:
//   Assumes tetrad is allocated to be 4*4.
//   Aligns 0-direction with u.
//   Aligns 3-direction with projection of k into space orthogonal to 0-direction.
//   Aligns 2-direction with projection of up into plane orthogonal to 0- and 3-directions.
//   Chooses 1-direction to make tetrad right-handed.
//   Assumes |det(g)| = 1 (as in Cartesian Kerr-Schild or Minkowski coordinates).
//   Generalizes 2012 ApJ 752 123 to case where up direction is not magnetic field, and changes
//       ordering of basis vectors.
void RadiationIntegrator::Tetrad(const double ucon[4], const double ucov[4], const double kcon[4],
    const double kcov[4], const double up_con[4], const double gcov[4][4], const double gcon[4][4],
    double tetrad[4][4]) const
{
  // Calculate fluid-frame frequency
  double omega = 0.0;
  for (int mu = 0; mu < 4; mu++)
    omega -= kcov[mu] * ucon[mu];

  // Calculate wavevector and up direction alignment
  double k_up_over_omega = 0.0;
  for (int mu = 0; mu < 4; mu++)
    k_up_over_omega += kcov[mu] * up_con[mu];
  k_up_over_omega /= omega;

  // Calculate velocity and up direction alignment
  double u_up_over_omega = 0.0;
  for (int mu = 0; mu < 4; mu++)
    u_up_over_omega += ucov[mu] * up_con[mu];
  u_up_over_omega /= omega;

  // Calculate components of unit vector in 0-direction
  for (int mu = 0; mu < 4; mu++)
    tetrad[0][mu] = ucon[mu];

  // Calculate components of unit vector in 3-direction
  for (int mu = 0; mu < 4; mu++)
    tetrad[3][mu] = kcon[mu] / omega - ucon[mu];

  // Calculate components of unit vector in 2-direction
  for (int mu = 0; mu < 4; mu++)
    tetrad[2][mu] = up_con[mu] - k_up_over_omega * tetrad[3][mu] + u_up_over_omega * kcon[mu];
  double norm = 0.0;
  for (int mu = 0; mu < 4; mu++)
    for (int nu = 0; nu < 4; nu++)
      norm += gcov[mu][nu] * tetrad[2][mu] * tetrad[2][nu];
  norm = std::sqrt(norm);
  for (int mu = 0; mu < 4; mu++)
    tetrad[2][mu] /= norm;

  // Calculate components of unit vector in 1-direction
  double tetrad_1_cov[4];
  tetrad_1_cov[0] = tetrad[0][1] * (tetrad[2][3] * tetrad[3][2] - tetrad[2][2] * tetrad[3][3])
      + tetrad[0][2] * (tetrad[2][1] * tetrad[3][3] - tetrad[2][3] * tetrad[3][1])
      + tetrad[0][3] * (tetrad[2][2] * tetrad[3][1] - tetrad[2][1] * tetrad[3][2]);
  tetrad_1_cov[1] = tetrad[0][0] * (tetrad[2][2] * tetrad[3][3] - tetrad[2][3] * tetrad[3][2])
      + tetrad[0][2] * (tetrad[2][3] * tetrad[3][0] - tetrad[2][0] * tetrad[3][3])
      + tetrad[0][3] * (tetrad[2][0] * tetrad[3][2] - tetrad[2][2] * tetrad[3][0]);
  tetrad_1_cov[2] = tetrad[0][0] * (tetrad[2][3] * tetrad[3][1] - tetrad[2][1] * tetrad[3][3])
      + tetrad[0][1] * (tetrad[2][0] * tetrad[3][3] - tetrad[2][3] * tetrad[3][0])
      + tetrad[0][3] * (tetrad[2][1] * tetrad[3][0] - tetrad[2][0] * tetrad[3][1]);
  tetrad_1_cov[3] = tetrad[0][0] * (tetrad[2][1] * tetrad[3][2] - tetrad[2][2] * tetrad[3][1])
      + tetrad[0][1] * (tetrad[2][2] * tetrad[3][0] - tetrad[2][0] * tetrad[3][2])
      + tetrad[0][2] * (tetrad[2][0] * tetrad[3][1] - tetrad[2][1] * tetrad[3][0]);
  for (int mu = 0; mu < 4; mu++)
  {
    tetrad[1][mu] = 0.0;
    for (int nu = 0; nu < 4; nu++)
      tetrad[1][mu] += gcon[mu][nu] * tetrad_1_cov[nu];
  }
  return;
}
