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

#ifndef _PID_CONTROLLER_H_
#define _PID_CONTROLLER_H_

#include <systemc-ams>

SC_MODULE(pid_controller)
{
  sca_lsf::sca_in e;
  sca_lsf::sca_out u;

  sca_lsf::sca_gain gain1;
  sca_lsf::sca_integ integ1;
  sca_lsf::sca_dot dot1;
  sca_lsf::sca_add add1, add2;

  pid_controller( sc_core::sc_module_name, double kp, double ki, double kd );

 private:
  sca_lsf::sca_signal sig_p, sig_i, sig_d, sig_pi;
};

#endif // _PID_CONTROLLER_H_
