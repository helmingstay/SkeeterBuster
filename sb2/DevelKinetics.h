//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.
#pragma once

// core enzyme kinetics model
inline double DevelKinetics(
    double avg_temp_k, bool to_kelvin,
    double ro25, 
    double dha, double dhh, 
    double rr, double thalf
){
    if (to_kelvin) {
        avg_temp_k += 273.15;
    }
    double dev_numer = (
        ro25*(avg_temp_k/298.0)
        *exp((dha/rr)*((1./298.)-(1./avg_temp_k)))
    );
    double dev_denom = 1+exp(
        (dhh/rr)
        *((1/thalf)-(1/avg_temp_k))
    );
    return (dev_numer/dev_denom)*24.;
}

// from here down just feeds parameters to the above
inline double DevelKineticsEgg(
    double avg_temp_k, bool to_kelvin=false
) {
    return DevelKinetics(
        avg_temp_k, to_kelvin,
        PbMsg.enzyme().ro25_embryonation(),
        PbMsg.enzyme().dha_embryonation(),
        PbMsg.enzyme().dhh_embryonation(),
        PbMsg.enzyme().r(),
        PbMsg.enzyme().thalf_embryonation()
    );
}

inline double DevelKineticsLarva(
    double avg_temp_k, bool to_kelvin = false
) {
    double ret =  DevelKinetics(
        avg_temp_k, to_kelvin, 
        PbMsg.enzyme().ro25_larval_development(),
        PbMsg.enzyme().dha_larval_development(),
        PbMsg.enzyme().dhh_larval_development(),
        PbMsg.enzyme().r(),
        PbMsg.enzyme().thalf_larval_development()
    );
    return ret;
}


inline double DevelKineticsPupa(
    double avg_temp_k, bool to_kelvin = false
) {
    double ret =  DevelKinetics(
        avg_temp_k, to_kelvin, 
        PbMsg.enzyme().ro25_pupal_development(),
        PbMsg.enzyme().dha_pupal_development(),
        PbMsg.enzyme().dhh_pupal_development(),
        PbMsg.enzyme().r(),
        PbMsg.enzyme().thalf_pupal_development()
    );
    return ret;
}

inline double DevelKineticsFemale(
    double avg_temp_k, bool to_kelvin=false
) {
    return DevelKinetics(
        avg_temp_k, to_kelvin,
        PbMsg.enzyme().ro25_gonotrophic_cycle(),
        PbMsg.enzyme().dha_gonotrophic_cycle(),
        PbMsg.enzyme().dhh_gonotrophic_cycle(),
        PbMsg.enzyme().r(),
        PbMsg.enzyme().thalf_gonotrophic_cycle()
    );
}
