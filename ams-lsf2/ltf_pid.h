//----------------------------------------------------------------------
//   Copyright 2009-2020 NXP
//   Copyright 2009-2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2015-2020 COSEDA Technologies GmbH
//   All Rights Reserved Worldwide
//
//   Licensed under the Apache License, Version 2.0 (the
//   "License"); you may not use this file except in
//   compliance with the License.  You may obtain a copy of
//   the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in
//   writing, software distributed under the License is
//   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//   CONDITIONS OF ANY KIND, either express or implied.  See
//   the License for the specific language governing
//   permissions and limitations under the License.
//----------------------------------------------------------------------

#ifndef _LTF_PID_H_
#define _LTF_PID_H_

#include <systemc-ams>

SC_MODULE(ltf_pid)
{
  sca_lsf::sca_in x;
  sca_lsf::sca_out y;

  sca_lsf::sca_ltf_nd* ltf1;

  ltf_pid( sc_core::sc_module_name nm, double kp_, double ki_, double kd_ );

 private:
  sca_util::sca_vector<double> num, den; // numerator and denominator coefficients
  double kp, ki, kd;
};

#endif // _LTF_PID_H_
