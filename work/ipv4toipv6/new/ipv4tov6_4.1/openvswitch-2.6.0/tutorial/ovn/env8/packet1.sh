#!/bin/bash
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -o xtrace

# input from local vif, lport1 (ofport 1)
# The destination MAC is not assigned to any host.
# expect to go out via l2gateway port (ofport 3)
ovs-appctl ofproto/trace br-int in_port=1,dl_src=00:00:00:00:00:01,dl_dst=00:00:00:00:00:03 -generate
