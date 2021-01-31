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

#include "pid_controller.h"

pid_controller::pid_controller( sc_core::sc_module_name, double kp, double ki, double kd )
: e("e"), u("u"), gain1("gain1", kp), integ1("integ1", ki), dot1("dot1", kd), add1("add1"),
  add2("add2"), sig_p("sig_p"), sig_i("sig_i"), sig_d("sig_d"), sig_pi("sig_pi")
{
  gain1.x(e);
  gain1.y(sig_p);

  integ1.x(e);
  integ1.y(sig_i);

  dot1.x(e);
  dot1.y(sig_d);

  add1.x1(sig_p);
  add1.x2(sig_i);
  add1.y(sig_pi);

  add2.x1(sig_pi);
  add2.x2(sig_d);
  add2.y(u);
}
