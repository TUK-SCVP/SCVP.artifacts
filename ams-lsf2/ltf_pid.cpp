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

#include "ltf_pid.h"

ltf_pid::ltf_pid( sc_core::sc_module_name nm, double kp_, double ki_, double kd_ )
: x("x"), y("y"), kp(kp_), ki(ki_), kd(kd_)
{
  // transfer function of entire system
  //
  //                  2
  //              kd s  + kp s + ki
  //  H(s)=  ------------------------------
  //          3           2
  //         s  + (10+kd)s  + (20+kp)s + ki

  num(0) = ki;
  num(1) = kp;
  num(2) = kd;
  den(0) = ki;
  den(1) = 20+kp;
  den(2) = 10+kd;
  den(3) = 1;

  ltf1 = new sca_lsf::sca_ltf_nd("ltf1", num ,den );
  ltf1->x(x);
  ltf1->y(y);
}
