/**
# Oscillating Film Simulation (`oscillatingFilm.c`)

Axisymmetric three-phase oscillating thin-film configuration with optional
viscoelastic stresses in each phase.

## Runtime Inputs

The case reads `key=value` pairs from `argv[1]` via `params.h`
(`case.params` when no argument is supplied). Common keys include:

- `MAXlevel`, `tmax`, `Ldomain`
- `Ohl`, `Ecl`, `Del` (lower medium)
- `Ohf`, `hf`, `Ecf`, `Def` (film medium)
- `Ohu`, `Ecu`, `Deu` (upper medium)
- `amp`, `lambda_wave`

## Outputs

- Restart file: `restart`
- Snapshots: `intermediate/snapshot-*`
- Log file: `log`
*/

#include "params.h"
#include "navier-stokes/centered.h"
#define FILTERED
#include "three-phase-nonCoalescing-viscoelastic.h"
#include "log-conform-viscoelastic.h"
#include "tension.h"

/**
## Numerical Tolerances

Adaptive criteria for interface, curvature, velocity, and conformation fields.
*/
#define fErr (1e-3) // error tolerance in VOF
#define KErr (1e-4) // error tolerance in KAPPA
#define VelErr (1e-2) // error tolerances in velocity
#define AErr (1e-3) // error tolerance in Conformation tensor
#define MINlevel 4 // minimum level
#define tsnap (1e-1)
#define tsnap2 (1e-2)


int MAXlevel;
double tmax;

/**
## Material Parameters

Lower medium (`l`), film medium (`f`), and upper medium (`u`) parameter sets.
*/
double Ohl, Ecl, Del;
double Ohf, hf, Ecf, Def;
double Ohu, Ecu, Deu;
double Ldomain;
double amp, lambda_wave, k_wave;

/**
## main()

Loads runtime parameters, initializes domain/material properties, and enters
the Basilisk event loop.
*/
int main(int argc, char const *argv[]) {

  char comm[80];
  sprintf (comm, "mkdir -p intermediate");
  system(comm);

  params_init_from_argv(argc, argv);

  MAXlevel = param_int("MAXlevel", 10);
  tmax = param_double("tmax", 2e2);

  // Lower medium
  Ohl = param_double("Ohl", 1e-1);
  Ecl = param_double("Ecl", 0.0);
  Del = param_double("Del", 0.0);

  // Film
  Ohf = param_double("Ohf", 1e0);
  hf = param_double("hf", 0.25);
  Ecf = param_double("Ecf", 0.1);
  Def = param_double("Def", 1e30);

  // Upper medium
  Ohu = param_double("Ohu", 1e-1);
  Ecu = param_double("Ecu", 0.0);
  Deu = param_double("Deu", 0.0);

  Ldomain = param_double("Ldomain", 1e0);
  if (Ldomain <= 0.0) {
    fprintf (ferr,
             "WARNING: Ldomain must be > 0. Resetting Ldomain to 1.0.\n");
    Ldomain = 1.0;
  }

  amp = param_double("amp", 2.5e-1);
  lambda_wave = param_double("lambda_wave", Ldomain);

  if (lambda_wave <= 0.0) {
    fprintf (ferr,
             "WARNING: lambda_wave must be > 0. Resetting lambda_wave to Ldomain (%g).\n",
             Ldomain);
    lambda_wave = Ldomain;
  }
  k_wave = 2.0*pi/lambda_wave;

  fprintf(ferr, "Level %d tmax %g. Ohl %3.2f, Ecl %3.2f, Del %3.2e, Ohf %3.2f, hf %3.2f, Ecf %3.2f, Def %4.3e, Ohu %3.2f, Ecu %3.2f, Deu %4.3e, amp %3.2e, lambda_wave %3.2f, k %3.2f\n", MAXlevel, tmax, Ohl, Ecl, Del, Ohf, hf, Ecf, Def, Ohu, Ecu, Deu, amp, lambda_wave, k_wave);

  L0=Ldomain;
  X0=-L0/2.; Y0=0.0;
  init_grid (1 << (6));

  // lower medium
  rho1 = 1e0; mu1 = Ohl; G1 = Ecl; lambda1 = Del;
  // upper medium
  rho2 = 1e0; mu2 = Ohu; G2 = Ecu; lambda2 = Deu;
  // film medium
  rho3 = 1e0; mu3 = Ohf; G3 = Ecf; lambda3 = Def;

  f1.sigma = 1e0;
  f2.sigma = 1e0;

  run();

}

/**
## init()

Initializes interfaces unless a restart snapshot is available.
*/
event init(t = 0){
  if (!restore (file = "restart")) {

    fraction(f1, -x + amp*cos(k_wave*y));
    fraction(f2, x - hf - amp*cos(k_wave*y));

  }
}

/**
## adapt()

Adaptive mesh refinement driven by interfaces, curvature, velocity, and
conformation fields.
*/
scalar KAPPA1[], KAPPA2[], trA[];
event adapt(i++){
  curvature(f1, KAPPA1);
  curvature(f2, KAPPA2);
  foreach(){
    trA[] = (conform_p.x.x[] + conform_p.y.y[])/2.0;
  }

  adapt_wavelet ((scalar *){f1, f2, KAPPA1, KAPPA2, u.x, u.y, trA},
  (double[]){fErr, fErr, KErr, KErr, VelErr, VelErr, AErr},
  MAXlevel, MINlevel);
}
/**
## writingFiles()

Writes periodic restart and snapshot files.
*/
event writingFiles (t = 0, t += tsnap; t <= tmax+tsnap) {
  dump (file = "restart");
  char nameOut[80];
  sprintf (nameOut, "intermediate/snapshot-%5.4f", t);
  dump (file = nameOut);
}

/**
## logWriting()

Logs kinetic energy and interface position diagnostics over time.
*/

scalar X1[];
scalar X2[];
static FILE * fp;

event logWriting (t = 0, t += tsnap2; t <= tmax+tsnap) {
  double ke = 0.;
  foreach (reduction(+:ke)){
    ke += (0.5*rho(f1[], f2[])*(sq(u.x[]) + sq(u.y[])))*sq(Delta);
  }

  position(f1, X1, {1, 0});
  position(f2, X2, {1, 0});

  if (pid() == 0){
    if (i == 0) {
      fprintf (ferr, "i dt t ke x1min x2max\n");
      fp = fopen ("log", "w");
      fprintf(fp, "Level %d tmax %g. Ohl %3.2f, Ecl %3.2f, Del %3.2e, Ohf %3.2f, hf %3.2f, Ecf %3.2f, Def %4.3e, Ohu %3.2f, Ecu %3.2f, Deu %4.3e, amp %3.2e, lambda_wave %3.2f, k %3.2f\n", MAXlevel, tmax, Ohl, Ecl, Del, Ohf, hf, Ecf, Def, Ohu, Ecu, Deu, amp, lambda_wave, k_wave);
      fprintf (fp, "i dt t ke x1min x2max\n");
    } else {
      fp = fopen ("log", "a");
    }
    fprintf (fp, "%d %g %g %g %g %g\n", i, dt, t, ke, statsf(X1).min, statsf(X2).max);
    fclose(fp);
    fprintf (ferr, "%d %g %g %g %g %g\n", i, dt, t, ke, statsf(X1).min, statsf(X2).max);
  }

  assert(ke > -1e-10);
  assert(ke < 1e2);
  // dump(file = "dumpTest");
}
