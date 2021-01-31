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

#ifndef _LTF_ND_FILTER_H_
#define _LTF_ND_FILTER_H_

#include <systemc-ams>

SC_MODULE(ltf_nd_filter)
{
  sca_lsf::sca_in x;
  sca_lsf::sca_out y;
  
  sca_lsf::sca_ltf_nd* ltf1;
  
  ltf_nd_filter( sc_core::sc_module_name nm );

 private:
  // numerator and denominator coefficients
  sca_util::sca_vector<double> num, den;
};

#endif // _LTF_ND_FILTER_H_
