/*
 * Copyright Université Rennes and Université Bretagne Sud
 * contributor(s) : Bertrand Le Gal   (2025-2026),
 *                  Abdallah Abdallah (2025-2026),
 *                  Camille  Monière  (2025-2026)
 *
 * bertrand.le-gal@univ-rennes.fr,
 * abdallah.abdallah@univ-ubs.fr,
 * camille.moniere@univ-ubs.fr
 *
 * This software is governed by the CeCILL license under French law and
 * abiding by the rules of distribution of free software.  You can  use,
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 */
#pragma once
#include <iostream>
#include <ostream>
#include <chrono>
//
//
//
//
//
class decoder {
public:
    decoder(const int _gf_, const int _n_, const int * frozen_symb);
    virtual ~decoder();
    virtual void execute(void * channel, uint16_t * decoded) = 0;
    virtual void setResult(const uint16_t * decoded) { }

public:
    virtual int64_t n_decoded_frames();
    virtual float  dec_avg_info_mbps();
    virtual float  dec_min_info_mbps();
    virtual float  dec_max_info_mbps();

    virtual float  dec_avg_coded_mbps();
    virtual float  dec_min_coded_mbps();
    virtual float  dec_max_coded_mbps();

    virtual float  dec_avg_latency();
    virtual float  dec_min_latency();
    virtual float  dec_max_latency();

    virtual int n ();
    virtual int k ();
    virtual int gf();

protected:
    virtual void dec_start();
    virtual void dec_stop ();

private:
    int64_t n_decoding;

	std::chrono::steady_clock::time_point t_start;
    double sum_exec_time;
    double min_exec_time;
    double max_exec_time;
    double last_exec_time;

protected:
    int GF;
    int N;
    int K;

    std::vector<int32_t> frozen;
    std::vector<int32_t> fiabilite;
};
//
//
//
//
//
