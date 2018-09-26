//=============================================================================================================
/**
* @file     mne.cpp
* @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     July, 2012
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
* @brief     Definition of the MNE Wrapper Class.
*
*/

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "mne.h"
#include <fiff/fiff.h>
#include <iostream>


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QFile>
#include <QDebug>


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace MNELIB;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

bool MNE::read_events(QString t_sEventName,
                      MatrixXi& events,
                      QString t_fileRawName)
{
    QFile t_EventFile;
    qint32 p;
    bool status = false;

    if (t_sEventName.size() == 0) {
        p = t_fileRawName.indexOf(".fif");
        if (p > 0) {
            t_sEventName = t_fileRawName.replace(p, 4, "-eve.fif");
        } else {
            printf("Raw file name does not end properly\n");
            return 0;
        }
//        events = mne_read_events(t_sEventName);

        t_EventFile.setFileName(t_sEventName);
        if(!MNE::read_events(t_EventFile, events)) {
            printf("Error while read events.\n");
            return false;
        }
        printf("Events read from %s\n",t_sEventName.toUtf8().constData());
    }
    else
    {
        // Binary file
        p = t_fileRawName.indexOf(".fif");
        if (p > 0) {
            t_EventFile.setFileName(t_sEventName);
            if(!MNE::read_events(t_EventFile, events)) {
                printf("Error while read events.\n");
                return false;
            }
            printf("Binary event file %s read\n",t_sEventName.toUtf8().constData());
        } else {
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

    return true;
}


//*************************************************************************************************************

bool MNE::read_events(QIODevice &p_IODevice, MatrixXi& eventlist)
{
    //
    // Open file
    //
    FiffStream::SPtr t_pStream(new FiffStream(&p_IODevice));

    if(!t_pStream->open()) {
        return false;
    }

    //
    //   Find the desired block
    //
    QList<FiffDirNode::SPtr> events = t_pStream->dirtree()->dir_tree_find(FIFFB_MNE_EVENTS);

    if (events.size() == 0)
    {
        printf("Could not find event data\n");
        return false;
    }

    qint32 k, nelem;
    fiff_int_t kind, pos;
    FiffTag::SPtr t_pTag;
    quint32* serial_eventlist_uint = NULL;
    qint32* serial_eventlist_int = NULL;

    for(k = 0; k < events[0]->nent(); ++k)
    {
        kind = events[0]->dir[k]->kind;
        pos  = events[0]->dir[k]->pos;
        if (kind == FIFF_MNE_EVENT_LIST)
        {
            t_pStream->read_tag(t_pTag,pos);
            if(t_pTag->type == FIFFT_UINT)
            {
                serial_eventlist_uint = t_pTag->toUnsignedInt();
                nelem = t_pTag->size()/4;
            }

            if(t_pTag->type == FIFFT_INT)
            {
                serial_eventlist_int = t_pTag->toInt();
                nelem = t_pTag->size()/4;
            }

            break;
        }
    }

    if(serial_eventlist_uint == NULL && serial_eventlist_int == NULL)
    {
        printf("Could not find any events\n");
        return false;
    }
    else
    {
        eventlist.resize(nelem/3,3);
        if(serial_eventlist_uint != NULL)
        {
            for(k = 0; k < nelem/3; ++k)
            {
                eventlist(k,0) = serial_eventlist_uint[k*3];
                eventlist(k,1) = serial_eventlist_uint[k*3+1];
                eventlist(k,2) = serial_eventlist_uint[k*3+2];
            }
        }

        if(serial_eventlist_int != NULL)
        {
            for(k = 0; k < nelem/3; ++k)
            {
                eventlist(k,0) = serial_eventlist_int[k*3];
                eventlist(k,1) = serial_eventlist_int[k*3+1];
                eventlist(k,2) = serial_eventlist_int[k*3+2];
            }
        }
    }

    return true;
}

//*************************************************************************************************************

MNEEpochDataList MNE::read_epochs(const FiffRawData& raw,
                                  const MatrixXi& events,
                                  const RowVectorXi& picks,
                                  float tmin,
                                  float tmax,
                                  qint32 event,
                                  double dEOGThreshold)
{
    MNEEpochDataList data;

    // Select the desired events
    qint32 count = 0;
    qint32 p;
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
    if (count > 0) {
        printf("%d matching events found\n",count);
    } else {
        printf("No desired events found.\n");
        return MNEEpochDataList();
    }

    fiff_int_t event_samp, from, to;
    fiff_int_t dropCount = 0;
    MatrixXd timesDummy;
    MatrixXd times;
    double min, max;

    MNEEpochData* epoch = Q_NULLPTR;

    int iChType = FIFFV_EOG_CH; //FIFFV_MEG_CH FIFFV_EEG_CH FIFFV_EOG_CH
    int iEOGChIdx = -1;

    for(int i = 0; i < raw.info.chs.size(); ++i) {
        if(raw.info.chs.at(i).kind == iChType) {
            iEOGChIdx = i;
            //qDebug() << "EOG channel found";
            break;
        }
    }

    if(iEOGChIdx == -1) {
        qDebug() << "MNE::read_epochs - No EOG channel found for epoch rejection";
    }

    for (p = 0; p < count; ++p) {
        // Read a data segment
        event_samp = events(selected(p),0);
        from = event_samp + tmin*raw.info.sfreq;
        to   = event_samp + floor(tmax*raw.info.sfreq + 0.5);

        epoch = new MNEEpochData();

        if(raw.read_raw_segment(epoch->epoch, timesDummy, from, to, picks)) {
            if (p == 0) {
                times.resize(1, to-from+1);
                for (qint32 i = 0; i < times.cols(); ++i)
                    times(0, i) = ((float)(from-event_samp+i)) / raw.info.sfreq;
            }

            epoch->event = event;
            epoch->tmin = ((float)(from)-(float)(raw.first_samp))/raw.info.sfreq;
            epoch->tmax = ((float)(to)-(float)(raw.first_samp))/raw.info.sfreq;

            if(iEOGChIdx >= 0 &&
               iEOGChIdx < epoch->epoch.rows() &&
               dEOGThreshold > 0.0) {
                RowVectorXd vecRow = epoch->epoch.row(iEOGChIdx);
                vecRow = vecRow.array() - vecRow(0);
                //vecRow = vecRow.array() - vecRow.mean();

                min = vecRow.minCoeff();
                max = vecRow.maxCoeff();

                //qDebug() << "std::fabs(min)" << std::fabs(min);
                //qDebug() << "std::fabs(max)" << std::fabs(max);

                //If absolute vaue of min or max if bigger than threshold -> reject
                if((std::fabs(min) > dEOGThreshold) || (std::fabs(max) > dEOGThreshold)) {
                    epoch->bReject = true;
                    dropCount++;
                    //qDebug() << "Epoch at sample" << event_samp << "rejected based on EOG channel";
                }
            }

            data.append(MNEEpochData::SPtr(epoch));//List takes ownwership of the pointer - no delete need
        } else {
            printf("Can't read the event data segments");
        }
    }

    qDebug() << "Read total of"<< data.size() <<"epochs and dropped"<< dropCount <<"of them";

    return data;
}


//*************************************************************************************************************

void MNE::setup_compensators(FiffRawData& raw,
                             fiff_int_t dest_comp,
                             bool keep_comp)
{
    qint32 k;

    // Set up projection
    if (raw.info.projs.size() == 0) {
        printf("No projector specified for these data\n");
    } else {
        // Activate the projection items
        for (k = 0; k < raw.info.projs.size(); ++k) {
            raw.info.projs[k].active = true;
        }

        printf("%d projection items activated\n",raw.info.projs.size());
        // Create the projector
//        fiff_int_t nproj = MNE::make_projector_info(raw.info, raw.proj); Using the member function instead
        fiff_int_t nproj = raw.info.make_projector(raw.proj);

        if (nproj == 0)  {
            printf("The projection vectors do not apply to these channels\n");
        } else {
            printf("Created an SSP operator (subspace dimension = %d)\n",nproj);
        }
    }

    // Set up the CTF compensator
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
            return;
        }
    }

}
