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

#include "ltf_nd_filter.h"

ltf_nd_filter::ltf_nd_filter( sc_core::sc_module_name nm )
: x("x"), y("y")
{
  num(0) = 1.0;   //               1
  den(0) = 20.0;  //  H(s)=  -------------
  den(1) = 10.0;  //          2
  den(2) = 1.0;   //         s  + 10s + 20
  
  ltf1 = new sca_lsf::sca_ltf_nd("ltf1", num ,den );
  ltf1->x(x);
  ltf1->y(y);
}
