touch ../scenario-details.txt
sumo -c osm.sumocfg --fcd-output ../trace.xml >> ../scenario-details.txt
cd ..
$SUMO_HOME/tools/traceExporter.py --fcd-input trace.xml --ns2mobility-output ns2-trace.tcl
