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

#include "ml.h"
#include "GRT.h"

#include <flext.h>

namespace ml
{
    
    typedef enum mlp_mode_
    {
        MLP_MODE_REGRESSION,
        MLP_MODE_CLASSIFICATION,
        MLP_NUM_MODES
    }
    mlp_mode;
    
    typedef enum mlp_layer_
    {
        MLP_LAYER_INPUT,
        MLP_LAYER_HIDDEN,
        MLP_LAYER_OUTPUT,
        MLP_NUM_LAYERS
    }
    mlp_layer;
    
    const GRT::UINT defaultNumInputDimensions = 2;
    const GRT::UINT defaultNumOutputDimensions = 1;
    const GRT::UINT defaultNumHiddenNeurons = 2;
    const mlp_mode defaultMode = MLP_MODE_REGRESSION;
    
    
    class ml_mlp : ml_base
    {
        FLEXT_HEADER_S(ml_mlp, ml_base, setup);
        
    public:
        ml_mlp()
        :
        numHiddenNeurons(defaultNumHiddenNeurons),
        inputActivationFunction((GRT::Neuron::ActivationFunctions)mlp.getInputLayerActivationFunction()),
        hiddenActivationFunction((GRT::Neuron::ActivationFunctions)mlp.getHiddenLayerActivationFunction()),
        outputActivationFunction((GRT::Neuron::ActivationFunctions)mlp.getOutputLayerActivationFunction()),
        mode(defaultMode)
        {
            std::string grt_version = mlp.getGRTVersion();
            post("ml.mlp: Multilayer Perceptron based on the GRT library version %s", grt_version.c_str());
            
            regressionData.setInputAndTargetDimensions(defaultNumInputDimensions, defaultNumOutputDimensions);
            classificationData.setNumDimensions(defaultNumInputDimensions);
            
            mlp.setMinChange(1.0e-2);
        }
        
        ~ml_mlp()
        {
            
        }
        
    protected:
        static void setup(t_classid c)
        {
            FLEXT_CADDATTR_SET(c, "mode", set_mode);
            FLEXT_CADDATTR_SET(c, "num_inputs", set_num_inputs);
            FLEXT_CADDATTR_SET(c, "num_outputs", set_num_outputs);
            FLEXT_CADDATTR_SET(c, "num_hidden", set_num_hidden);
            FLEXT_CADDATTR_SET(c, "min_epochs", set_min_epochs);
            FLEXT_CADDATTR_SET(c, "max_epochs", set_max_epochs);
            FLEXT_CADDATTR_SET(c, "min_change", set_min_change);
            FLEXT_CADDATTR_SET(c, "training_rate", set_training_rate);
            FLEXT_CADDATTR_SET(c, "momentum", set_momentum);
            FLEXT_CADDATTR_SET(c, "gamma", set_gamma);
            FLEXT_CADDATTR_SET(c, "multi_threaded_training", set_multi_threaded_training);
            FLEXT_CADDATTR_SET(c, "null_rejection", set_null_rejection);
            FLEXT_CADDATTR_SET(c, "null_rejection_coeff", set_null_rejection_coeff);
            FLEXT_CADDATTR_SET(c, "input_activation_function", set_input_activation_function);
            FLEXT_CADDATTR_SET(c, "hidden_activation_function", set_hidden_activation_function);
            FLEXT_CADDATTR_SET(c, "output_activation_function", set_output_activation_function);
            FLEXT_CADDATTR_SET(c, "rand_training_iterations", set_rand_training_iterations);
            FLEXT_CADDATTR_SET(c, "use_validation_set", set_use_validation_set);
            FLEXT_CADDATTR_SET(c, "validation_set_size", set_validation_set_size);
            FLEXT_CADDATTR_SET(c, "randomize_training_order", set_randomise_training_order);
            FLEXT_CADDATTR_SET(c, "enable_scaling", set_enable_scaling);
            
            FLEXT_CADDATTR_GET(c, "mode", get_mode);
            FLEXT_CADDATTR_GET(c, "num_inputs", get_num_inputs);
            FLEXT_CADDATTR_GET(c, "num_outputs", get_num_outputs);
            FLEXT_CADDATTR_GET(c, "num_hidden", get_num_hidden);
            FLEXT_CADDATTR_GET(c, "min_epochs", get_min_epochs);
            FLEXT_CADDATTR_GET(c, "max_epochs", get_max_epochs);
            FLEXT_CADDATTR_GET(c, "min_change", get_min_change);
            FLEXT_CADDATTR_GET(c, "training_rate", get_training_rate);
            FLEXT_CADDATTR_GET(c, "momentum", get_momentum);
            FLEXT_CADDATTR_GET(c, "gamma", get_gamma);
            FLEXT_CADDATTR_GET(c, "multi_threaded_training", get_multi_threaded_training);
            FLEXT_CADDATTR_GET(c, "null_rejection", get_null_rejection);
            FLEXT_CADDATTR_GET(c, "null_rejection_coeff", get_null_rejection_coeff);
            FLEXT_CADDATTR_GET(c, "input_activation_function", get_input_activation_function);
            FLEXT_CADDATTR_GET(c, "hidden_activation_function", get_hidden_activation_function);
            FLEXT_CADDATTR_GET(c, "output_activation_function", get_output_activation_function);
            FLEXT_CADDATTR_GET(c, "rand_training_iterations", get_rand_training_iterations);
            FLEXT_CADDATTR_GET(c, "use_validation_set", get_use_validation_set);
            FLEXT_CADDATTR_GET(c, "validation_set_size", get_validation_set_size);
            FLEXT_CADDATTR_GET(c, "randomize_training_order", get_randomise_training_order);
            FLEXT_CADDATTR_GET(c, "enable_scaling", get_enable_scaling);
        }
        
        // Methods
        virtual void add(int argc, const t_atom *argv);
        virtual void save(const t_symbol *path) const;
        virtual void load(const t_symbol *path);
        virtual void train();
        virtual void clear();
        virtual void classify(int argc, const t_atom *argv);
        virtual void usage();
        
        // Attribute Setters
        void set_mode(int mode);
        void set_num_inputs(int num_inputs);
        void set_num_outputs(int num_outputs);
        void set_num_hidden(int num_hidden);
        void set_min_epochs(int min_epochs);
        void set_max_epochs(int max_epochs);
        void set_min_change(float min_change);
        void set_training_rate(float training_rate);
        void set_momentum(float momentum); //
        void set_gamma(float gamma);
        void set_multi_threaded_training(bool multi_threaded_training);
        void set_null_rejection(bool null_rejection);
        void set_null_rejection_coeff(float null_rejection_coeff);
        void set_input_activation_function(int activation_function);
        void set_hidden_activation_function(int activation_function);
        void set_output_activation_function(int activation_function);
        void set_rand_training_iterations(int rand_training_iterations);
        void set_use_validation_set(bool use_validation_set);
        void set_validation_set_size(int validation_set_size);
        void set_randomise_training_order(bool randomise_training_order);
        void set_enable_scaling(bool enable_scaling);
        
        // Attribute Getters
        void get_mode(int &mode) const;
        void get_num_inputs(int &num_inputs) const;
        void get_num_outputs(int &num_outputs) const;
        void get_num_hidden(int &num_hidden) const;
        void get_min_epochs(int &min_epochs) const;
        void get_max_epochs(int &max_epochs) const;
        void get_min_change(float &min_change) const;
        void get_training_rate(float &training_rate) const;
        void get_momentum(float &momentum) const; //
        void get_gamma(float &gamma) const;
        void get_multi_threaded_training(bool &multi_threaded_training) const;
        void get_null_rejection(bool &null_rejection) const;
        void get_null_rejection_coeff(float &null_rejection_coeff) const;
        void get_input_activation_function(int &activation_function) const;
        void get_hidden_activation_function(int &activation_function) const;
        void get_output_activation_function(int &activation_function) const;
        void get_rand_training_iterations(int &rand_training_iterations) const;
        void get_use_validation_set(bool &use_validation_set) const;
        void get_validation_set_size(int &validation_set_size) const;
        void get_randomise_training_order(bool &randomise_training_order) const;
        void get_enable_scaling(bool &enable_scaling) const;
                
    private:
        void set_activation_function(int activation_function, mlp_layer layer);
        
        // Method wrappers

        // Attribute wrappers
        FLEXT_CALLVAR_I(get_mode, set_mode);
        FLEXT_CALLVAR_I(get_num_inputs, set_num_inputs);
        FLEXT_CALLVAR_I(get_num_outputs, set_num_outputs);
        FLEXT_CALLVAR_I(get_num_hidden, set_num_hidden);
        FLEXT_CALLVAR_I(get_min_epochs, set_min_epochs);
        FLEXT_CALLVAR_I(get_max_epochs, set_max_epochs);
        FLEXT_CALLVAR_F(get_min_change, set_min_change);
        FLEXT_CALLVAR_F(get_training_rate, set_training_rate);
        FLEXT_CALLVAR_F(get_momentum, set_momentum);
        FLEXT_CALLVAR_F(get_gamma, set_gamma);
        FLEXT_CALLVAR_B(get_multi_threaded_training, set_multi_threaded_training);
        FLEXT_CALLVAR_B(get_null_rejection, set_null_rejection);
        FLEXT_CALLVAR_F(get_null_rejection_coeff, set_null_rejection_coeff);
        FLEXT_CALLVAR_I(get_input_activation_function, set_input_activation_function);
        FLEXT_CALLVAR_I(get_hidden_activation_function, set_hidden_activation_function);
        FLEXT_CALLVAR_I(get_output_activation_function, set_output_activation_function);
        FLEXT_CALLVAR_I(get_rand_training_iterations, set_rand_training_iterations);
        FLEXT_CALLVAR_B(get_use_validation_set, set_use_validation_set);
        FLEXT_CALLVAR_I(get_validation_set_size, set_validation_set_size);
        FLEXT_CALLVAR_B(get_randomise_training_order, set_randomise_training_order);
        FLEXT_CALLVAR_B(get_enable_scaling, set_enable_scaling);

        // Instance variables
        GRT::MLP mlp;
        GRT::LabelledRegressionData regressionData;
        GRT::LabelledClassificationData classificationData;
        GRT::UINT numHiddenNeurons;
        GRT::Neuron::ActivationFunctions inputActivationFunction;
        GRT::Neuron::ActivationFunctions hiddenActivationFunction;
        GRT::Neuron::ActivationFunctions outputActivationFunction;
        mlp_mode mode;
        
    };
    
    // Utility functions
    std::string get_symbol_as_string(const t_symbol *symbol)
    {
        const char *c_string = flext::GetAString(symbol);
        std::string cpp_string;
        
        if (c_string == NULL)
        {
            error("symbol was NULL");
        }
        else
        {
            cpp_string.assign(c_string);
        }
        
        return cpp_string;
    }
    
    // Attribute setters
    void ml_mlp::set_mode(int mode)
    {
        if (mode > MLP_NUM_MODES)
        {
            error("mode must be between 0 and %d", MLP_NUM_MODES - 1);
            return;
        }
        
        if (mode == MLP_MODE_CLASSIFICATION)
        {
            set_num_outputs(1);
        }
        
        this->mode = (mlp_mode)mode;
    }
    
    void ml_mlp::set_num_inputs(int num_inputs)
    {
        if (num_inputs < 0)
        {
            error("number of inputs must be greater than zero");
        }
        
        bool success = false;
        
        if (mode == MLP_MODE_CLASSIFICATION)
        {
            success = classificationData.setNumDimensions(num_inputs);
        }
        else if (mode == MLP_MODE_REGRESSION)
        {
            success = regressionData.setInputAndTargetDimensions(num_inputs, regressionData.getNumTargetDimensions());
        }
        
        if (success == false)
        {
            error("unable to set input or target dimensions");
            return;
        }
    }
    
    void ml_mlp::set_num_outputs(int num_outputs)
    {
        if (num_outputs < 0)
        {
            error("number of outputs must be greater than zero");
        }
        
        if (mode == MLP_MODE_CLASSIFICATION && num_outputs > 1)
        {
            error("for classification mode, number of outputs must be 1, for multidimensional output switch mode to %d", MLP_MODE_REGRESSION);
            return;
        }
        
        if (mode == MLP_MODE_REGRESSION)
        {
            bool success = regressionData.setInputAndTargetDimensions(regressionData.getNumInputDimensions(), num_outputs);
           
            if (success == false)
            {
                error("unable to set input and target dimensions");
                return;
            }
        }
    }
    
    void ml_mlp::set_num_hidden(int num_hidden)
    {
        this->numHiddenNeurons = num_hidden;
    }
    
    void ml_mlp::set_min_epochs(int min_epochs)
    {
        bool success = mlp.setMinNumEpochs(min_epochs);
        
        if (success == false)
        {
            error("unable to set min_epochs, hint: should be greater than 0");
        }
    }

    void ml_mlp::set_max_epochs(int max_epochs)
    {
        mlp.setMaxNumEpochs(max_epochs);
    }
    
    void ml_mlp::set_min_change(float min_change)
    {
        bool success = mlp.setMinChange(min_change);
        
        if (success == false)
        {
            error("unable to set min_change, hint: should be greater than 0");
        }
    }
    
    void ml_mlp::set_training_rate(float training_rate)
    {
        bool success = mlp.setTrainingRate(training_rate);
        
        if (success == false)
        {
            error("unable to set training_rate, hint: should be between 0-1");
        }
    }
    
    void ml_mlp::set_momentum(float momentum)
    {
        bool success = mlp.setMomentum(momentum);
        
        if (success == false)
        {
            error("unable to set momentum, hint: should be between 0-1");
        }
    }
    
    void ml_mlp::set_gamma(float gamma)
    {
        bool success = mlp.setGamma(gamma);
        
        if (success == false)
        {
            error("unable to set gamma");
        }
    }

    void ml_mlp::set_multi_threaded_training(bool multi_threaded_training)
    {
        error("function not implemented");
//        bool success = mlp.setUseMultiThreadingTraining(multi_threaded_training);
//        
//        if (success == false)
//        {
//            error("unable to set multi_threaded_training");
//        }
    }

    void ml_mlp::set_null_rejection(bool null_rejection)
    {
        bool success = mlp.setNullRejection(null_rejection);
        
        if (success == false)
        {
            error("unable to set null_rejection");
        }
    }

    void ml_mlp::set_null_rejection_coeff(float null_rejection_coeff)
    {
        bool success = mlp.setNullRejectionCoeff(null_rejection_coeff);
        
        if (success == false)
        {
            error("unable to set null_rejection_coeff, hint: should be greater than 0");
        }
    }
    
    void ml_mlp::set_activation_function(int activation_function, mlp_layer layer)
    {
        if (mlp.validateActivationFunction(activation_function) == false)
        {
            error("activation function %d is invalid, hint should be between 0-%d", activation_function, GRT::Neuron::NUMBER_OF_ACTIVATION_FUNCTIONS - 1);
            return;
        }
        
        GRT::Neuron::ActivationFunctions activation_function_ = (GRT::Neuron::ActivationFunctions)activation_function;
        
        switch (layer)
        {
            case MLP_LAYER_INPUT:
                inputActivationFunction = activation_function_;
                break;
            case MLP_LAYER_HIDDEN:
                hiddenActivationFunction = activation_function_;
                break;
            case MLP_LAYER_OUTPUT:
                outputActivationFunction = activation_function_;
                break;
            default:
                error("no activation function for layer: %d", layer);
                return;
        }
        post("activation function set to %s", mlp.activationFunctionToString(activation_function_).c_str());
    }
    
    void ml_mlp::set_input_activation_function(int activation_function)
    {
        set_activation_function(activation_function, MLP_LAYER_INPUT);
    }
    
    void ml_mlp::set_hidden_activation_function(int activation_function)
    {
        set_activation_function(activation_function, MLP_LAYER_HIDDEN);
    }
    
    void ml_mlp::set_output_activation_function(int activation_function)
    {
        set_activation_function(activation_function, MLP_LAYER_OUTPUT);
    }
    
    void ml_mlp::set_rand_training_iterations(int rand_training_iterations)
    {
        bool success = mlp.setNumRandomTrainingIterations(rand_training_iterations);
        
        if (success == false)
        {
            error("unable to set rand_training_iterations, hint: should be greater than 0");
        }
    }
    
    void ml_mlp::set_use_validation_set(bool use_validation_set)
    {
        bool success = mlp.setUseValidationSet(use_validation_set);
        
        if (success == false)
        {
            error("unable to set use_validation_set, hint: should be 0 or 1");
        }
    }
    
    void ml_mlp::set_validation_set_size(int validation_set_size)
    {
        bool success = mlp.setValidationSetSize(validation_set_size);
        
        if (success == false)
        {
            error("unable to set validation_set_size, hint: should be between 0-100");
        }
    }

    void ml_mlp::set_randomise_training_order(bool randomise_training_order)
    {
        bool success = mlp.setRandomiseTrainingOrder(randomise_training_order);
        
        if (success == false)
        {
            error("unable to set randomise_training_order, hint: should be 0 or 1");
        }
    }
    
    void ml_mlp::set_enable_scaling(bool enable_scaling)
    {
        bool success = mlp.enableScaling(enable_scaling);
        
        if (success == false)
        {
            error("unable to set randomise_training_order, hint: should be 0 or 1");
        }
    }

    // Attribute getters
    void ml_mlp::get_mode(int &mode) const
    {
        mode = this->mode;
    }
    
    void ml_mlp::get_num_inputs(int &num_inputs) const
    {
        if (mode == MLP_MODE_CLASSIFICATION)
        {
            num_inputs = classificationData.getNumDimensions();
        }
        else if (mode == MLP_MODE_REGRESSION)
        {
            num_inputs = regressionData.getNumInputDimensions();
        }
    }
    
    void ml_mlp::get_num_outputs(int &num_outputs) const
    {
        if (mode == MLP_MODE_CLASSIFICATION)
        {
            num_outputs = defaultNumOutputDimensions;
        }
        else if (mode == MLP_MODE_REGRESSION)
        {
            num_outputs = regressionData.getNumTargetDimensions();
        }
    }
    
    void ml_mlp::get_num_hidden(int &num_hidden) const
    {
        num_hidden = this->numHiddenNeurons;
    }
    
    void ml_mlp::get_min_epochs(int &min_epochs) const
    {
        min_epochs = mlp.getMinNumEpochs();
    }

    void ml_mlp::get_max_epochs(int &max_epochs) const
    {
        max_epochs = mlp.getMaxNumEpochs();
    }

    void ml_mlp::get_min_change(float &min_change) const
    {
        min_change = mlp.getMinChange();
    }
    
    void ml_mlp::get_training_rate(float &training_rate) const
    {
        training_rate = mlp.getTrainingRate();
    }
    
    void ml_mlp::get_momentum(float &momentum) const
    {
        momentum = mlp.getMomentum();
    }
    
    void ml_mlp::get_gamma(float &gamma) const
    {
        gamma = mlp.getGamma();
    }
    
    void ml_mlp::get_multi_threaded_training(bool &multi_threaded_training) const
    {
        error("function not implemented");
    }
    
    void ml_mlp::get_null_rejection(bool &null_rejection) const
    {
        null_rejection = mlp.getNullRejectionEnabled();
    }
    
    void ml_mlp::get_null_rejection_coeff(float &null_rejection_coeff) const
    {
        null_rejection_coeff = mlp.getNullRejectionCoeff();
    }
    
    void ml_mlp::get_input_activation_function(int &activation_function) const
    {
        activation_function = inputActivationFunction;
    }
    
    void ml_mlp::get_hidden_activation_function(int &activation_function) const
    {
        activation_function = hiddenActivationFunction;
    }
    
    void ml_mlp::get_output_activation_function(int &activation_function) const
    {
        activation_function = outputActivationFunction;
    }
    
    void ml_mlp::get_rand_training_iterations(int &rand_training_iterations) const
    {
        rand_training_iterations = mlp.getNumRandomTrainingIterations();
    }

    void ml_mlp::get_use_validation_set(bool &use_validation_set) const
    {
        use_validation_set = mlp.getUseValidationSet();
    }
    
    void ml_mlp::get_validation_set_size(int &validation_set_size) const
    {
        validation_set_size = mlp.getValidationSetSize();
    }
    
    void ml_mlp::get_randomise_training_order(bool &randomise_training_order) const
    {
        randomise_training_order = mlp.getRandomiseTrainingOrder();
    }

    void ml_mlp::get_enable_scaling(bool &enable_scaling) const
    {
        enable_scaling = mlp.getScalingEnabled();
    }
    
    // Methods
    void ml_mlp::add(int argc, const t_atom *argv)
    {
        if (argc < 2)
        {
            error("invalid input length, must contain at least 2 values");
            return;
        }
        
        GRT::UINT numInputDimensions = mode == MLP_MODE_CLASSIFICATION ? classificationData.getNumDimensions() : regressionData.getNumInputDimensions();
        GRT::UINT numOutputDimensions = mode == MLP_MODE_CLASSIFICATION ? 1 : regressionData.getNumTargetDimensions();
        GRT::UINT combinedVectorSize = numInputDimensions + numOutputDimensions;
        
        
        if (argc < 0 || (unsigned)argc != combinedVectorSize)
        {
            numInputDimensions = argc - numOutputDimensions;
            
            if (numInputDimensions < 1)
            {
                error("invalid input length, expected at least %d", numOutputDimensions + 1);
                return;
            }
            post("new input vector size, adjusting num_inputs to %d", numInputDimensions);
            set_num_inputs(numInputDimensions);
        }
        
        GRT::VectorDouble inputVector(numInputDimensions);
        GRT::VectorDouble targetVector(numOutputDimensions);
        
        for (uint32_t index = 0; index < (unsigned)argc; ++index)
        {
            float value = GetAFloat(argv[index]);

            if (index < numOutputDimensions)
            {
                targetVector[index] = value;
            }
            else
            {
                inputVector[index - numOutputDimensions] = value;
            }
        }

        if (mode == MLP_MODE_CLASSIFICATION)
        {
            GRT::UINT label = (GRT::UINT)targetVector[0];
            
            if ((double)label != targetVector[0])
            {
                error("class label must be an integer");
                return;
            }
            
            if (label == 0)
            {
                error("class label must be non-zero");
                return;
            }
            classificationData.addSample((GRT::UINT)targetVector[0], inputVector);
        }
        else if (mode == MLP_MODE_REGRESSION)
        {
            regressionData.addSample(inputVector, targetVector);   
        }
    }

    void ml_mlp::save(const t_symbol *path) const
    {
        if (regressionData.getNumSamples() == 0 && regressionData.getNumSamples() == 0)
        {
            error("no observations added, use 'add' to add training data");
            return;
        }
        
        std::string file_path = get_symbol_as_string(path);
        
        if (file_path.empty())
        {
            error("path string is empty");
            return;
        }
        
        bool success = false;
        
        if (mode == MLP_MODE_CLASSIFICATION)
        {
            success = classificationData.saveDatasetToFile(file_path);
        }
        else if (mode == MLP_MODE_REGRESSION)
        {
            success = regressionData.saveDatasetToFile(file_path);
        }
        
        if (!success)
        {
            error("unable to save training data to path: %s", file_path.c_str());
        }
    }
    
    void ml_mlp::load(const t_symbol *path)
    {
        std::string file_path = get_symbol_as_string(path);
        
        if (file_path.empty())
        {
            error("path string is empty");
            return;
        }

        bool success = false;
        
        if (mode == MLP_MODE_CLASSIFICATION)
        {
            success = classificationData.loadDatasetFromFile(file_path);
        }
        else if (mode == MLP_MODE_REGRESSION)
        {
            success = regressionData.loadDatasetFromFile(file_path);
        }
        
        if (!success)
        {
            error("unable to load training data from path: %s", file_path.c_str());
        }
    }
    
    void ml_mlp::train()
    {
        GRT::UINT numSamples = mode == MLP_MODE_CLASSIFICATION ? classificationData.getNumSamples() : regressionData.getNumSamples();
        
        if (numSamples == 0)
        {
            error("no observations added, use 'add' to add training data");
            return;
        }
        
        bool success = false;
        
        if (mode == MLP_MODE_CLASSIFICATION)
        {
            mlp.init(
                     classificationData.getNumDimensions(),
                     numHiddenNeurons,
                     classificationData.getNumClasses(),
                     inputActivationFunction,
                     hiddenActivationFunction,
                     outputActivationFunction
                     );
            success = mlp.train(classificationData);
        }
        else if (mode == MLP_MODE_REGRESSION)
        {
            mlp.init(
                     regressionData.getNumInputDimensions(),
                     numHiddenNeurons,
                     regressionData.getNumTargetDimensions(),
                     inputActivationFunction,
                     hiddenActivationFunction,
                     outputActivationFunction
                     );
            success = mlp.train(regressionData);
        }

        if (!success)
        {
            error("training failed");
            return;
        }
        
        if ((mode == MLP_MODE_CLASSIFICATION && mlp.getClassificationModeActive() == false) || (mode == MLP_MODE_REGRESSION && mlp.getRegressionModeActive() == false))
        {
            error("mode mismatch");
            return;
        }
        
        t_atom a_num_classes;
        
        SetInt(a_num_classes, mlp.getNumClasses());
        ToOutAnything(1, s_train, 1, &a_num_classes);
    }
    
    void ml_mlp::clear()
    {
        t_atom status;
        SetBool(status, true);
        
        regressionData.clear();
        classificationData.clear();
        mlp.clear();
        
        ToOutAnything(1, s_cleared, 1, &status);
    }
    
    void ml_mlp::classify(int argc, const t_atom *argv)
    {
        GRT::UINT numSamples = mode == MLP_MODE_CLASSIFICATION ? classificationData.getNumSamples() : regressionData.getNumSamples();

        if (numSamples == 0)
        {
            error("no observations added, use 'add' to add training data");
            return;
        }

        if (mlp.getTrained() == false)
        {
            error("model has not been trained, use 'train' to train the model");
            return;
        }
        
        GRT::UINT numInputNeurons = mlp.getNumInputNeurons();
        GRT::VectorDouble query(numInputNeurons);
        
        if (argc < 0 || (unsigned)argc != numInputNeurons)
        {
            error("invalid input length, expected %d, got %d", numInputNeurons, argc);
        }

        for (uint32_t index = 0; index < (uint32_t)argc; ++index)
        {
            double value = GetAFloat(argv[index]);
            query[index] = value;
        }
        
        bool success = mlp.predict(query);
        
        if (success == false)
        {
            error("unable to classify input");
            return;
        }
        
        if (mlp.getClassificationModeActive())
        {
            GRT::VectorDouble likelihoods = mlp.getClassLikelihoods();
            GRT::vector<GRT::UINT> labels = classificationData.getClassLabels();
            GRT::UINT classification = mlp.getPredictedClassLabel();
            
            if (likelihoods.size() != labels.size())
            {
                error("labels / likelihoods size mismatch");
            }
            else
            {
                AtomList estimates;

                for (uid_t count = 0; count < labels.size(); ++count)
                {
                    t_atom label_a;
                    t_atom likelihood_a;
                    
                    // Need to call SetDouble() first or label_a gets corrupted. Bug in Flext?
                    SetDouble(&likelihood_a, likelihoods[count]);
                    SetInt(label_a, labels[count]);
                    
                    estimates.Append(label_a);
                    estimates.Append(likelihood_a);
                }
                ToOutAnything(1, s_estimates, estimates);
            }
                 
            ToOutInt(0, classification);
        }
        else if (mlp.getRegressionModeActive())
        {
            GRT::VectorDouble regressionData = mlp.getRegressionData();
            GRT::VectorDouble::size_type numOutputDimensions = regressionData.size();
            
            if (numOutputDimensions != mlp.getNumOutputNeurons())
            {
                error("invalid output dimensions: %d", numOutputDimensions);
                return;
            }
            
            AtomList result;
            
            for (uint32_t index = 0; index < numOutputDimensions; ++index)
            {
                t_atom value_a;
                double value = regressionData[index];
                SetFloat(value_a, value);
                result.Append(value_a);
            }
            
            ToOutList(0, result);
        }
    }
    
    void ml_mlp::usage()
    {
        post("%s", ML_POST_SEP);
        post("Attributes:");
        post("%s", ML_POST_SEP);
        post("mode:\tinteger setting mode of the MLP, %d for regression and %d for classification (default %d)",
             MLP_MODE_REGRESSION, MLP_MODE_CLASSIFICATION, defaultMode);
        post("num_inputs:\tinteger setting number of neurons in the input layer of the MLP (default %d)", defaultNumInputDimensions);
        post("num_outputs:\tinteger setting number of neurons in the output layer of the MLP (default %d)", defaultNumOutputDimensions);
        post("num_hidden:\tinteger setting number of neurons in the hidden layer of the MLP (default %d)", defaultNumHiddenNeurons);
        post("min_epochs:\tinteger setting the minimum number of training iterations (default 10)");
        post("max_epochs:\tinteger setting the maximum number of training iterations (default 100)");
        post("min_change:\tfloating point value setting the minimum change that must be achieved between two training epochs for the training to continue (default 1.0e-5)");
        post("training_rate:\tfloating point value used to update the weights at each step of the stochastic gradient descent (default 0.1)");
        post("momentum:\tfloating point value setting the momentum of the MLP (default 0.5)");
        post("gamma:\tfloating point value setting the gamma of the MLP (default 2.0)");
        post("null_rejection:\tinteger (0 or 1) toggling NULL rejection off or on, when 'on' classification results below the NULL-rejection threshold will be discarded (default 1)");
        post("null_rejection_coeff:\tfloating point value setting a multiplier for the NULL-rejection threshold (default 0.9)");
        post("input_activation_function:\tinteger determining the activation function for the input layer, 0:LINEAR, 1:SIGMOID, 2:BIPOLAR_SIGMOID (default LINEAR)");
        post("hidden_activation_function:\tinteger determining the activation function for the hidden layer, 0:LINEAR, 1:SIGMOID, 2:BIPOLAR_SIGMOID (default LINEAR)");
        post("output_activation_function:\tinteger determining the activation function for the output layer, 0:LINEAR, 1:SIGMOID, 2:BIPOLAR_SIGMOID (default LINEAR)");
        post("rand_training_iterations:\tinteger setting the number of random training iterations (default 10)");
        post("use_validation_set:\tinteger (0 or 1) determining whether or not to use a validation training set (default 1)");
        post("validation_set_size:\tinteger integer determining the size of the validation set (default 20)");
        post("randomize_training_order:\tinteger (0 or 1) determining whether or not to randomize the training order (default 0)");
        post("enable_scaling:\tinteger (0 or 1) determining whether or not values are automatically scaled (default 1)");
        post("%s", ML_POST_SEP);
        post("Methods:");
        post("%s", ML_POST_SEP);
        post("add:\tlist comprising a class id followed by n features; <class> <feature 1> <feature 2> etc when in classification mode or N output values followed by M input values when in regression mode, where N is determined by the num_outputs attribute");
        post("save:\tsave training examples, first argument gives path to save location");
        post("load:\tload training examples, first argument gives path to the load location");
        post("train:\ttrain the MLP based on vectors added with 'add'");
        post("clear:\tclear the stored training data and model");
        post("classify:\tgive the class of the input feature vector provided as a list in classification mode or the regression outputs in regression mode");
        post("help:\tpost this usage statement to the console");
        post("%s", ML_POST_SEP);
    }
    
    FLEXT_LIB("ml.mlp", ml_mlp);
    
} //namespace ml

