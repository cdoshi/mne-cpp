//=============================================================================================================
/**
* @file     main.cpp
* @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     July, 2013
*
* @section  LICENSE
*
* Copyright (C) 2012, Christoph Dinh and Matti Hamalainen. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that
* the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
*       following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
*       the following disclaimer in the documentation and/or other materials provided with the distribution.
*     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
*       to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*
* @brief    Example of the computation of a test_mne_cluster_eval
*
*/


//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include <fs/label.h>
#include <fs/surface.h>
#include <fs/annotationset.h>

#include <fiff/fiff_evoked.h>
#include <fiff/fiff.h>
#include <mne/mne.h>

#include <mne/mne_epoch_data_list.h>

#include <mne/mne_sourceestimate.h>
#include <inverse/minimumNorm/minimumnorm.h>

#include <utils/mnemath.h>

#include <iostream>

#include <fstream>


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QGuiApplication>
#include <QSet>
#include <QElapsedTimer>

//#define BENCHMARK


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace MNELIB;
using namespace FSLIB;
using namespace FIFFLIB;
using namespace INVERSELIB;
using namespace UTILSLIB;


//*************************************************************************************************************
//=============================================================================================================
// MAIN
//=============================================================================================================

//=============================================================================================================
/**
* The function main marks the entry point of the program.
* By default, main has the storage class extern.
*
* @param [in] argc (argument count) is an integer that indicates how many arguments were entered on the command line when the program was started.
* @param [in] argv (argument vector) is an array of pointers to arrays of character objects. The array objects are null-terminated strings, representing the arguments that were entered on the command line when the program was started.
* @return the value that was set to exit() (which is 0 if exit() is called via quit()).
*/
int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    //
    // calculate the average
    //

    for(qint32 numAverages = 50; numAverages <= 50; numAverages += 1)
    {

        for(qint32 it = 0; it <= 30; ++it)
        {

            QFile t_fileFwd("D:/Data/MEG/mind006/mind006_051209_auditory01_raw-oct-6p-fwd.fif");
            QFile t_fileCov("D:/Data/MEG/mind006/mind006_051209_auditory01_raw-cov.fif");
            QFile t_fileRaw("D:/Data/MEG/mind006/mind006_051209_auditory01_raw.fif");
            QString t_sEventName = "D:/Data/MEG/mind006/mind006_051209_auditory01_raw-eve.fif";
            AnnotationSet t_annotationSet("mind006", 2, "aparc.a2009s", "D:/Data/subjects");

            qint32 event = 1;

            float tmin = -0.2f;
            float tmax = 0.4f;

            bool keep_comp = false;
            fiff_int_t dest_comp = 0;
            bool pick_all  = true;

            qint32 k, p;

            //
            //   Setup for reading the raw data
            //
            FiffRawData raw(t_fileRaw);

            RowVectorXi picks;
            if (pick_all)
            {
                //
                // Pick all
                //
                picks.resize(raw.info.nchan);

                for(k = 0; k < raw.info.nchan; ++k)
                    picks(k) = k;
                //
            }
            else
            {
                QStringList include;
                include << "STI 014";
                bool want_meg   = true;
                bool want_eeg   = false;
                bool want_stim  = false;

        //        picks = Fiff::pick_types(raw.info, want_meg, want_eeg, want_stim, include, raw.info.bads);
                picks = raw.info.pick_types(want_meg, want_eeg, want_stim, include, raw.info.bads);//prefer member function
            }

            QStringList ch_names;
            for(k = 0; k < picks.cols(); ++k)
                ch_names << raw.info.ch_names[picks(0,k)];

            //
            //   Set up projection
            //
            if (raw.info.projs.size() == 0)
                printf("No projector specified for these data\n");
            else
            {
                //
                //   Activate the projection items
                //
                for (k = 0; k < raw.info.projs.size(); ++k)
                    raw.info.projs[k].active = true;

                printf("%d projection items activated\n",raw.info.projs.size());
                //
                //   Create the projector
                //
        //        fiff_int_t nproj = MNE::make_projector_info(raw.info, raw.proj); Using the member function instead
                fiff_int_t nproj = raw.info.make_projector(raw.proj);

                if (nproj == 0)
                {
                    printf("The projection vectors do not apply to these channels\n");
                }
                else
                {
                    printf("Created an SSP operator (subspace dimension = %d)\n",nproj);
                }
            }

            //
            //   Set up the CTF compensator
            //
        //    qint32 current_comp = MNE::get_current_comp(raw.info);
            qint32 current_comp = raw.info.get_current_comp();
            if (current_comp > 0)
                printf("Current compensation grade : %d\n",current_comp);

            if (keep_comp)
                dest_comp = current_comp;

            if (current_comp != dest_comp)
            {
                qDebug() << "This part needs to be debugged";
                if(MNE::make_compensator(raw.info, current_comp, dest_comp, raw.comp))
                {
        //            raw.info.chs = MNE::set_current_comp(raw.info.chs,dest_comp);
                    raw.info.set_current_comp(dest_comp);
                    printf("Appropriate compensator added to change to grade %d.\n",dest_comp);
                }
                else
                {
                    printf("Could not make the compensator\n");
                    return 0;
                }
            }
            //
            //  Read the events
            //
            QFile t_EventFile;
            MatrixXi events;
            if (t_sEventName.size() == 0)
            {
                p = t_fileRaw.fileName().indexOf(".fif");
                if (p > 0)
                {
                    t_sEventName = t_fileRaw.fileName().replace(p, 4, "-eve.fif");
                }
                else
                {
                    printf("Raw file name does not end properly\n");
                    return 0;
                }
        //        events = mne_read_events(t_sEventName);

                t_EventFile.setFileName(t_sEventName);
                MNE::read_events(t_EventFile, events);
                printf("Events read from %s\n",t_sEventName.toUtf8().constData());
            }
            else
            {
                //
                //   Binary file
                //
                p = t_fileRaw.fileName().indexOf(".fif");
                if (p > 0)
                {
                    t_EventFile.setFileName(t_sEventName);
                    if(!MNE::read_events(t_EventFile, events))
                    {
                        printf("Error while read events.\n");
                        return 0;
                    }
                    printf("Binary event file %s read\n",t_sEventName.toUtf8().constData());
                }
                else
                {
                    //
                    //   Text file
                    //
                    printf("Text file %s is not supported jet.\n",t_sEventName.toUtf8().constData());
        //            try
        //                events = load(eventname);
        //            catch
        //                error(me,mne_omit_first_line(lasterr));
        //            end
        //            if size(events,1) < 1
        //                error(me,'No data in the event file');
        //            end
        //            //
        //            //   Convert time to samples if sample number is negative
        //            //
        //            for p = 1:size(events,1)
        //                if events(p,1) < 0
        //                    events(p,1) = events(p,2)*raw.info.sfreq;
        //                end
        //            end
        //            //
        //            //    Select the columns of interest (convert to integers)
        //            //
        //            events = int32(events(:,[1 3 4]));
        //            //
        //            //    New format?
        //            //
        //            if events(1,2) == 0 && events(1,3) == 0
        //                fprintf(1,'The text event file %s is in the new format\n',eventname);
        //                if events(1,1) ~= raw.first_samp
        //                    error(me,'This new format event file is not compatible with the raw data');
        //                end
        //            else
        //                fprintf(1,'The text event file %s is in the old format\n',eventname);
        //                //
        //                //   Offset with first sample
        //                //
        //                events(:,1) = events(:,1) + raw.first_samp;
        //            end
                }
            }

        //
        //    Select the desired events
        //
        qint32 count = 0;
        MatrixXi selected = MatrixXi::Zero(1, events.rows());
        for (p = 0; p < events.rows(); ++p)
        {
            if (events(p,1) == 0 && events(p,2) == event)
            {
                selected(0,count) = p;
                ++count;
            }
        }
        selected.conservativeResize(1, count);
        if (count > 0)
            printf("%d matching events found\n",count);
        else
        {
            printf("No desired events found.\n");
            return 0;
        }


        fiff_int_t event_samp, from, to;
        MatrixXd timesDummy;

        MNEEpochDataList data;

        MNEEpochData* epoch = NULL;

        MatrixXd times;

        for (p = 0; p < count; ++p)
        {
            //
            //       Read a data segment
            //
            event_samp = events(selected(p),0);
            from = event_samp + tmin*raw.info.sfreq;
            to   = event_samp + floor(tmax*raw.info.sfreq + 0.5);

            epoch = new MNEEpochData();

            if(raw.read_raw_segment(epoch->epoch, timesDummy, from, to, picks))
            {
                if (p == 0)
                {
                    times.resize(1, to-from+1);
                    for (qint32 i = 0; i < times.cols(); ++i)
                        times(0, i) = ((float)(from-event_samp+i)) / raw.info.sfreq;
                }

                epoch->event = event;
                epoch->tmin = ((float)(from)-(float)(raw.first_samp))/raw.info.sfreq;
                epoch->tmax = ((float)(to)-(float)(raw.first_samp))/raw.info.sfreq;

                data.append(MNEEpochData::SPtr(epoch));//List takes ownwership of the pointer - no delete need
            }
            else
            {
                printf("Can't read the event data segments");
                return 0;
            }
        }

        if(data.size() > 0)
        {
            printf("Read %d epochs, %d samples each.\n",data.size(),(qint32)data[0]->epoch.cols());

            //DEBUG
            std::cout << data[0]->epoch.block(0,0,10,10) << std::endl;
            qDebug() << data[0]->epoch.rows() << " x " << data[0]->epoch.cols();

            std::cout << times.block(0,0,1,10) << std::endl;
            qDebug() << times.rows() << " x " << times.cols();
        }

            //Option 1
            VectorXi vecSel(numAverages);
            srand (time(NULL)); // initialize random seed

            for(qint32 i = 0; i < vecSel.size(); ++i)
            {
                qint32 val = rand() % data.size();
                vecSel(i) = val;
            }

//            //Option 3 Newest
//            VectorXi vecSel(10);

//            vecSel << 0, 96, 80, 55, 66, 25, 26, 2, 55, 58, 6, 88;

            std::cout << "Select following epochs to average:\n" << vecSel << std::endl;

            QString sSelFile = QString("aveInfo_%1_%2.txt").arg(numAverages).arg(it);
            std::ofstream selFile(sSelFile.toLatin1().constData());
            if (selFile.is_open())
            {
              selFile << vecSel << '\n';
            }


            FiffEvoked evoked = data.average(raw.info, tmin*raw.info.sfreq, floor(tmax*raw.info.sfreq + 0.5), vecSel);


            //########################################################################################
            // Source Estimate

            double snr = 1.0f;//0.1f;//1.0f;//3.0f;//0.1f;//3.0f;
            QString method("dSPM"); //"MNE" | "dSPM" | "sLORETA"

            QString t_sFileNameClusteredInv("");
            QString t_sFileNameStc = QString("mind006_051209_auditory01_%1_ave_it_%2.stc").arg(numAverages).arg(it);

            // Parse command line parameters
            for(qint32 i = 0; i < argc; ++i)
            {
                if(strcmp(argv[i], "-snr") == 0 || strcmp(argv[i], "--snr") == 0)
                {
                    if(i + 1 < argc)
                        snr = atof(argv[i+1]);
                }
                else if(strcmp(argv[i], "-method") == 0 || strcmp(argv[i], "--method") == 0)
                {
                    if(i + 1 < argc)
                        method = QString::fromUtf8(argv[i+1]);
                }
                else if(strcmp(argv[i], "-inv") == 0 || strcmp(argv[i], "--inv") == 0)
                {
                    if(i + 1 < argc)
                        t_sFileNameClusteredInv = QString::fromUtf8(argv[i+1]);
                }
                else if(strcmp(argv[i], "-stc") == 0 || strcmp(argv[i], "--stc") == 0)
                {
                    if(i + 1 < argc)
                        t_sFileNameStc = QString::fromUtf8(argv[i+1]);
                }
            }

            double lambda2 = 1.0 / pow(snr, 2);
            qDebug() << "Start calculation with: SNR" << snr << "; Lambda" << lambda2 << "; Method" << method << "; stc:" << t_sFileNameStc;

            MNEForwardSolution t_Fwd(t_fileFwd);
            if(t_Fwd.isEmpty())
                return 1;

            FiffCov noise_cov(t_fileCov);

            // regularize noise covariance
            noise_cov = noise_cov.regularize(evoked.info, 0.05, 0.05, 0.1, true);

            //
            // Cluster forward solution;
            //
            MatrixXd D;
            MNEForwardSolution t_clusteredFwd = t_Fwd.cluster_forward_solution(t_annotationSet, 20, D, noise_cov, evoked.info);

            //
            // make an inverse operators
            //
            FiffInfo info = evoked.info;


            MNEInverseOperator inverse_operator_clustered(info, t_clusteredFwd, noise_cov, 0.2f, 0.8f);

            //
            // save clustered inverse
            //
            if(!t_sFileNameClusteredInv.isEmpty())
            {
                QFile t_fileClusteredInverse(t_sFileNameClusteredInv);
                inverse_operator_clustered.write(t_fileClusteredInverse);
            }

            //
            // Compute inverse solution
            //
            MinimumNorm minimumNormClustered(inverse_operator_clustered, lambda2, method);

            MNESourceEstimate sourceEstimateClustered = minimumNormClustered.calculateInverse(evoked);

            if(sourceEstimateClustered.isEmpty())
                return 1;

            //Push Estimate
            if(!t_sFileNameStc.isEmpty())
            {
                QFile t_fileClusteredStc(t_sFileNameStc);
                sourceEstimateClustered.write(t_fileClusteredStc);
            }
        }
    }

    return a.exec();
}
