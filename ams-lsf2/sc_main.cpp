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

#include <systemc-ams>

#include "pid_controller.h"
#include "ltf_nd_filter.h"
#include "ltf_pid.h"

int sc_main(int argc, char* argv[])
{
  sc_core::sc_set_time_resolution(1.0, sc_core::SC_FS);
     
  sca_lsf::sca_signal in, error, out1, out2, pid_out;

  double init_value = 0.0;
  double offset = 1.0;
  double amplitude = 0.0;
  double frequency = 0.0;
  double phase = 0.0;
  sca_core::sca_time delay = sca_core::sca_time(10, sc_core::SC_MS);

  sca_lsf::sca_source src("src", init_value, offset, amplitude,
                          frequency, phase,delay ); // step of 1 unit at t=10ms
    src.y(in);
    src.set_timestep(10, sc_core::SC_MS);

  sca_lsf::sca_sub sub("sub");
    sub.x1(in);
    sub.x2(out1);
    sub.y(error);
    
  pid_controller pid("pid", 350.0, 300.0, 50.0); 
    pid.e(error);
    pid.u(pid_out);
  
  ltf_nd_filter plant("plant1");
    plant.x(pid_out);
    plant.y(out1);

  ltf_pid pidc("pidc", 350.0, 300.0, 50.0);
    pidc.x(in);
    pidc.y(out2);

  // tracing
  sca_util::sca_trace_file* atf = sca_util::sca_create_tabular_trace_file("pid_controller");
  sca_util::sca_trace(atf, in, "in");
  sca_util::sca_trace(atf, out1, "out1");
  sca_util::sca_trace(atf, out2, "out2");

  std::cout << "Simulation started..." << std::endl;

  sc_core::sc_start(2.0, sc_core::SC_SEC);

  std::cout << "Simulation finished." << std::endl;

  sca_util::sca_close_tabular_trace_file(atf);

  return 0;
}
