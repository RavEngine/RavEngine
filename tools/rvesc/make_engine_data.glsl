
EngineData make_engine_data(EngineData_Internal constants){
    EngineData data;
    data.viewProj = constants.viewProj;
    data.viewOnly = constants.viewOnly;
    data.projOnly = constants.projOnly;

    return data;
}