// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2002-2012.
//
// This software is released under a three-clause BSD license:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of any author or any participating institution
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
// For a full list of authors, refer to the file AUTHORS.
// --------------------------------------------------------------------------
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL ANY OF THE AUTHORS OR THE CONTRIBUTING
// INSTITUTIONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// --------------------------------------------------------------------------
// $Maintainer: Timo Sachsenberg $
// $Authors: Marc Sturm $
// --------------------------------------------------------------------------

#ifndef OPENMS_VISUAL_VISUALIZER_DIGESTIONVISUALIZER_H
#define OPENMS_VISUAL_VISUALIZER_DIGESTIONVISUALIZER_H

//OpenMS
#include <OpenMS/VISUAL/VISUALIZER/BaseVisualizer.h>
#include <OpenMS/VISUAL/VISUALIZER/BaseVisualizerGUI.h>
#include <OpenMS/METADATA/Digestion.h>


namespace OpenMS
{
  /**
      @brief Class that displays all meta information of digestion objects.

      This class provides all functionality to view the meta information of an object of type Digestion.
  */
  class OPENMS_GUI_DLLAPI DigestionVisualizer :
    public BaseVisualizerGUI,
    public BaseVisualizer<Digestion>
  {
    Q_OBJECT

public:

    ///Constructor
    DigestionVisualizer(bool editable = false, QWidget * parent = 0);

public slots:

    //Docu in base class
    void store();

protected slots:

    ///Undo the changes made in the GUI.
    void undo_();

protected:

    ///@name Edit fields and buttons
    //@{
    QLineEdit * treatmenttype_;
    QTextEdit * treatmentcomment_;
    QLineEdit * digestionenzyme_;
    QLineEdit * digestiontime_;
    QLineEdit * digestiontemperature_;
    QLineEdit * digestionPH_;
    //@}

    //Docu in base class
    void update_();
  };

}
#endif
