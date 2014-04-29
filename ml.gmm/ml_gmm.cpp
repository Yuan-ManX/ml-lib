/*
 * ml-lib, a machine learning library for Max and Pure Data
 * Copyright (C) 2013 Carnegie Mellon University
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ml_classification.h"

namespace ml
{
    // Utility functions
    
    
    // Class declaration
    class ml_gmm : ml_classification
    {
        FLEXT_HEADER_S(ml_gmm, ml, setup);
        
    public:
        ml_gmm()
        {
            post("ml.gmm: Gaussian Mixture Models based on the GRT library version %s", get_grt_version().c_str());
            set_scaling(default_scaling);
        }
        
        ~ml_gmm()
        {
            
        }
        
    protected:
        static void setup(t_classid c)
        {
            // Flext attribute set messages
            FLEXT_CADDATTR_SET(c, "num_mixture_models", set_num_mixture_models);
            
            // Flext attribute get messages
            FLEXT_CADDATTR_GET(c, "num_mixture_models", get_num_mixture_models);
            
            // Flext method messages
            
            // Associate this Flext class with a certain help file prefix
            DefineHelp(c,"ml.gmm");
        }
        
        // Methods
        void usage();
        
        // Attribute Setters
        void set_num_mixture_models(int type);
        
        // Attribute Getters
        void get_num_mixture_models(int &type) const;
        
        // Pure virtual method implementations
        GRT::Classifier &get_Classifier_instance();
        const GRT::Classifier &get_Classifier_instance() const;
        
    private:
        // Flext method wrappers
        
        // Flext attribute wrappers
        FLEXT_CALLVAR_I(get_num_mixture_models, set_num_mixture_models);
        
        // Instance variables
        GRT::GMM gmm;
    };
    
    // Utility functions
    
    
    // Attribute setters
    void ml_gmm::set_num_mixture_models(int num_mixture_models)
    {
        gmm.setNumMixtureModels(num_mixture_models);
    }
    
    
    // Attribute getters
    void ml_gmm::get_num_mixture_models(int &num_mixture_models) const
    {
        flext::error("function not implemented");
    }
    
    // Methods
    
    void ml_gmm::usage()
    {
        post("%s", ML_POST_SEP);
        post("Attributes:");
        post("%s", ML_POST_SEP);
        post("scaling:\tinteger (0 or 1) sets whether values are automatically scaled (default 1)");
        post("probs:\tinteger (0 or 1) determing whether probabilities are sent from the right outlet");
        post("null_rejection:\tinteger (0 or 1) toggling NULL rejection off or on, when 'on' classification results below the NULL-rejection threshold will be discarded (default 1)");
        post("null_rejection_coeff:\tfloating point value setting a multiplier for the NULL-rejection threshold (default 0.9)");
        post("num_mixture_models:\tinteger (n > 0) sets the number of mixture models used for class (default 2)");
        post("%s", ML_POST_SEP);
        post("Methods:");
        post("%s", ML_POST_SEP);
        post("add:\tlist comprising a class id followed by n features; <class> <feature 1> <feature 2> etc");
        post("save:\tsave training examples, first argument gives path to save location");
        post("load:\tload training examples, first argument gives path to the load location");
        post("train:\ttrain the MLP based on vectors added with 'add'");
        post("clear:\tclear the stored training data and data_typel");
        post("map:\tgive the regression value for the input feature vector");
        post("help:\tpost this usage statement to the console");
        post("%s", ML_POST_SEP);
        
    }
    
    // Implement pure virtual methods
    GRT::Classifier &ml_gmm::get_Classifier_instance()
    {
        return gmm;
    }
    
    const GRT::Classifier &ml_gmm::get_Classifier_instance() const
    {
        return gmm;
    }
    
    FLEXT_LIB("ml.gmm", ml_gmm);
    
} //namespace ml


