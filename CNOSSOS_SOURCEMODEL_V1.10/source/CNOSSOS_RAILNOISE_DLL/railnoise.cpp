Classes:

RailwayVehicle
	char	vehicle_category
	double	axles
	char	brake_type
	char	wheel_measure_type

Track
	char	track_base
	char	roughness
	char	pad_type
	char	additional_measures
	char	joints
	char	curvature

Source
	double	height
	int[]	source_types


Globals:
double	Tref	// hours
int		r0		// reference value for roughness [µm]
int		v0		// speed at which aerodynamic noise is dominant [km/h]


Enumerations:
running_conditions = (constant_speed = 1, idling = 2);
physical_source_types = (rolling = 1, traction = 2, aerodynamic = 3);


Conversion
	double wavelength /* cm */ (double speed /* km/h */, frequency /* Hz */)
	{
		return speed / (frequency * 0.036);
	}

	double erg(double x) {
		return pow(10, x / 10);
	}
	
	double dB(double x) {
		return 10 * log(x);
	}


Calculation	// IV-1
/*
j: track section index
t: index for vehicle types on the track section
s: index for train speed
r: index for running conditions
p: physical source types
i: octaafband
*/
for(j = 0; j < NUM_TRACK_SECTIONS; j++) {
	for (t = 0; t < NUM_VEHICLE_TYPES; t++) {
		for (s = 0; s < NUM_SPEEDS; s++) {
			for (r = 0; r < NUM_RUNNING_CONDITIONS; r++) {
				for (p = 0; p < NUM_SOURCE_TYPES; p++) {
					LwEqLine = calculate_3_2(t, s, r, p);
					for (i = 0; i < NUM_OCTAVE_BANDS; i++) {
						LwEqTdir[i] += erg(LwEqLine[i]);
					}
				}
			}
		}
	}
}
return dB(LwEqTdir);


double[] calculate_3_2(j, t, s, r, p)	// 3.2 traffic flow per source condition
/*
ψ: vertical angle
φ: horizontal angle
*/
ψ = get_vertical_angle(?); // << src=input?
φ = get_horizontal_angle(?); // << src=input?
Lw0dir = calculate_3_3(p, ψ, φ);
Q = get_num_vehicles(j, t, s, r); // << src=?
if (r == 1) {	// IV-2
	/*
	Q: avg number of vehicles per hour on the track section for vehicle type t, average train speed s and running condition r
	v: speed [km/h] on track section j for vehicle type t and average train speed s
	*/
	V = get_vehicle_speed(j, t, s);
	return Lw0dir + 10 * log(Q / (1000 * v));
} else if (r == 2) {	// IV-3
	/*
	T: Overall time for idling during Tref
	Q: Number of idling vehicles per Tref
	L: Section length for idling
	*/
	T = get_idling_time(j);
	L = get_section_length(j); // idling only?
	return Lw0dir + 10 * log(T / (Tref * L));
}


double[] calculate_3_3(?)	// 3.3 directional sound power level
Lw0 = 0;
switch(p) {
	case rolling:
		Lw0 = calculate_3_4(?);
		break;
	case traction:
		Lw0 = calculate_3_5(?);
		break;
	case aerodynamic:
		Lw0 = calculate_3_6(?);
		break;
}
ΔLwDirVert = calculate_3_3_1(?);
ΔLwDirHorz = calculate_3_3_2(?);
return Lw0 + ΔLwDirVert + ΔLwDirHorz;	// IV-4


double[] calculate_3_3_1(?) // 3.3.1 vertical directivity correction
if (h == 0.5) {
	/*
	fci: centre band frequency (1/3 octave)
	*/
	fci = get_centre_band_frequency(?);
	return abs( (40 / 3) * (2/3 * sin(2 * ψ) - sin(ψ) * log((fci + 600) / 200)) ); // IV-15
} else if (h == 4.0) {
	if (ψ < 0) {
		return 10 * log(pow(cos(ψ), 2)); // IV-16
	} else {
		return 0; // IV-17
	}
}


double[] calculate_3_3_2(φ) // 3.3.2 horizontal directivity correction
return 10 * log(0.01 + 0.99 * pow(sin(φ), 2));



double[] calculate_3_4(?)	// 3.4 Rolling noise; only for h==0.5
return calculate_3_4_1(?);


double[] calculate_3_4_1(?)	// 3.4.1 + 3.4.2 Rolling noise Sound power
/*
Na: number of axles
*/
Na = get_number_of_axles(vehicle_category);
LrTot = calculate_3_4_3(?);
LhTr = lookup_transfer_track(?);
LwTr = LrTot + LhTr + dB(Na);
LhVeh = lookup_transfer_vehicle(?);
LwVeh = LrTot + LhVeh + dB(Na);
LhVehSup = lookup_transfer_superstructure(?);
LwVehSup = LrTot + LhVehSup + dB(Na);
ΔLsqueal = calculate_3_4_4(R);
ΔLbridge = calculate_3_4_5(?);
return dB(erg(LwTr) + erg(LwVeh) + erg(LwVehSup)) + ΔLsqueal + ΔLbridge;


double[] calculate_3_4_3(track)	// 3.4.3 Roughness and impact
/*
LrRough:		Combined roughness level of the rail and wheel
LrImpact:		Impact noise (crossings, switches and junctions)
LrTr:			Roughness level of the rail
LrVeh:			Roughness level of the wheel
A3:				Contact filter expressed depending on rail and wheel type and load
n				Joint density [m-1]
LrImpactSingle	Impact roughness level for a single impact
*/
n = get_joint_density(track);
v = get_speed(?);
lookup_roughness(track, v, &LrTr, &LrVeh, &A3, &LrImpactSingle);
LrRough = dB(erg(LrTr) + erg(LrVeh)) + A3;
LrImpact = LrImpactSingle + dB(n / 0.01);
return dB(erg(LrRough) + erg(LrImpact));


double calculate_3_4_4(R)	// 3.4.4 Squeal
/*
R:	radius of the curve [m]
*/
if (R < 300)
	return 8;
else (R < 500)
	return 5
else
	return 0;


double calculate_3_4_5(bridge_type)	// 3.4.5 Bridge
return lookup_bridge(bridge_type);



double[] calculate_3_5(vehicle)	// 3.5 Traction noise; h=0.5|4.0
/*
r: running condition
*/
r = get_running_condition(vehicle);
Ltraction = lookup_traction(r);
return Ltraction - 3;



double[] calculate_3_6(vehicle)	// 3.6 Aerodynamic noise; h=0.5|4.0
/*
Lw0(v0):	Source power for rolling noise at speed v0 km/h
vts:		Vehicle speed
v0: 		Speed at which aerodynamic noise is dominant (= 250 km/h)
α1,
α2:		Coefficients for aerodynamic noise (default = 50)
*/
vts = get_vehicle_speed(vehicle);
if (vts <= 200) {
	return 0;
} else {
	lookup_aerodynamic_coefficients(&α1, &α2);
	Lw0v0 = calculate_3_4(?, v0);
	if (h == 0.5) {
		return Lw0v0 + α1 * log(vts / v0);
	} else if (h == 4.0) {
		return Lw0(v0) + α2 * log(vts / v0);
	}
}





////////////////////////////////////////////////////////////////////////////////////////////////////
// Lookup functions


// appendix IV-D


// appendix IV-E
void lookup_aerodynamic_coefficients(double* α1, α2) {
	α1 = 50;
	α2 = 50;
}

