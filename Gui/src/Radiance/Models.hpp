static StormGraph::IModel* model_button( StormGraph::IGraphicsDriver* driver, StormGraph::IMaterial* material, float width, float height )
{
    li::List<StormGraph::MeshCreationInfo*> meshes;

    float button_coords[] =
    {
        -8.0f, -8.0f, 0.0f,
        -8.0f, height + 8.0f, 0.0f,
        width + 8.0f, -8.0f, 0.0f,
        width + 8.0f, -8.0f, 0.0f,
        -8.0f, height + 8.0f, 0.0f,
        width + 8.0f, height + 8.0f, 0.0f,
    };

    float button_uvs[] =
    {
        0.0f, 0.0f,
        0.0f, 0.40625f,
        1.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 0.40625f,
        1.0f, 0.40625f,
    };

    StormGraph::MeshCreationInfo button_info;
    button_info.format = StormGraph::MeshFormat::triangleList;
    button_info.layout = StormGraph::MeshLayout::linear;
    button_info.numVertices = 6;
    button_info.numIndices = 0;
    button_info.coords = button_coords;
    button_info.normals = 0;
    button_info.uvs = button_uvs;
    button_info.indices = 0;
    button_info.material = material;
    meshes.add( &button_info );

    return driver->createModelFromMemory( "button", meshes );
}
static StormGraph::IModel* model_button_highlight( StormGraph::IGraphicsDriver* driver, StormGraph::IMaterial* material, float width, float height )
{
    li::List<StormGraph::MeshCreationInfo*> meshes;

    float highlight_coords[] =
    {
        -8.0f, -8.0f, 0.0f,
        -8.0f, height + 8.0f, 0.0f,
        width + 8.0f, -8.0f, 0.0f,
        width + 8.0f, -8.0f, 0.0f,
        -8.0f, height + 8.0f, 0.0f,
        width + 8.0f, height + 8.0f, 0.0f,
    };

    float highlight_uvs[] =
    {
        0.0f, 0.5f,
        0.0f, 0.90625f,
        1.0f, 0.5f,
        1.0f, 0.5f,
        0.0f, 0.90625f,
        1.0f, 0.90625f,
    };

    StormGraph::MeshCreationInfo highlight_info;
    highlight_info.format = StormGraph::MeshFormat::triangleList;
    highlight_info.layout = StormGraph::MeshLayout::linear;
    highlight_info.numVertices = 6;
    highlight_info.numIndices = 0;
    highlight_info.coords = highlight_coords;
    highlight_info.normals = 0;
    highlight_info.uvs = highlight_uvs;
    highlight_info.indices = 0;
    highlight_info.material = material;
    meshes.add( &highlight_info );

    return driver->createModelFromMemory( "button_highlight", meshes );
}
static StormGraph::IModel* model_input( StormGraph::IGraphicsDriver* driver, StormGraph::IMaterial* material, float width, float height )
{
    li::List<StormGraph::MeshCreationInfo*> meshes;

    float input_coords[] =
    {
        -8.0f, -8.0f, 0.0f,
        -8.0f, height + 8.0f, 0.0f,
        width + 8.0f, -8.0f, 0.0f,
        width + 8.0f, -8.0f, 0.0f,
        -8.0f, height + 8.0f, 0.0f,
        width + 8.0f, height + 8.0f, 0.0f,
    };

    float input_uvs[] =
    {
        0.0f, 0.0f,
        0.0f, 0.203125f,
        1.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 0.203125f,
        1.0f, 0.203125f,
    };

    StormGraph::MeshCreationInfo input_info;
    input_info.format = StormGraph::MeshFormat::triangleList;
    input_info.layout = StormGraph::MeshLayout::linear;
    input_info.numVertices = 6;
    input_info.numIndices = 0;
    input_info.coords = input_coords;
    input_info.normals = 0;
    input_info.uvs = input_uvs;
    input_info.indices = 0;
    input_info.material = material;
    meshes.add( &input_info );

    return driver->createModelFromMemory( "input", meshes );
}
static StormGraph::IModel* model_input_highlight( StormGraph::IGraphicsDriver* driver, StormGraph::IMaterial* material, float width, float height )
{
    li::List<StormGraph::MeshCreationInfo*> meshes;

    float input_coords[] =
    {
        -8.0f, -8.0f, 0.0f,
        -8.0f, height + 8.0f, 0.0f,
        width + 8.0f, -8.0f, 0.0f,
        width + 8.0f, -8.0f, 0.0f,
        -8.0f, height + 8.0f, 0.0f,
        width + 8.0f, height + 8.0f, 0.0f,
    };

    float input_uvs[] =
    {
        0.0f, 0.203125f,
        0.0f, 0.40625f,
        1.0f, 0.203125f,
        1.0f, 0.203125f,
        0.0f, 0.40625f,
        1.0f, 0.40625f,
    };

    StormGraph::MeshCreationInfo input_info;
    input_info.format = StormGraph::MeshFormat::triangleList;
    input_info.layout = StormGraph::MeshLayout::linear;
    input_info.numVertices = 6;
    input_info.numIndices = 0;
    input_info.coords = input_coords;
    input_info.normals = 0;
    input_info.uvs = input_uvs;
    input_info.indices = 0;
    input_info.material = material;
    meshes.add( &input_info );

    return driver->createModelFromMemory( "input_highlight", meshes );
}
static StormGraph::IModel* model_input_saturate( StormGraph::IGraphicsDriver* driver, StormGraph::IMaterial* material, float width, float height )
{
    li::List<StormGraph::MeshCreationInfo*> meshes;

    float input_coords[] =
    {
        -8.0f, -8.0f, 0.0f,
        -8.0f, height + 8.0f, 0.0f,
        width + 8.0f, -8.0f, 0.0f,
        width + 8.0f, -8.0f, 0.0f,
        -8.0f, height + 8.0f, 0.0f,
        width + 8.0f, height + 8.0f, 0.0f,
    };

    float input_uvs[] =
    {
        0.0f, 0.40625f,
        0.0f, 0.609375f,
        1.0f, 0.40625f,
        1.0f, 0.40625f,
        0.0f, 0.609375f,
        1.0f, 0.609375f,
    };

    StormGraph::MeshCreationInfo input_info;
    input_info.format = StormGraph::MeshFormat::triangleList;
    input_info.layout = StormGraph::MeshLayout::linear;
    input_info.numVertices = 6;
    input_info.numIndices = 0;
    input_info.coords = input_coords;
    input_info.normals = 0;
    input_info.uvs = input_uvs;
    input_info.indices = 0;
    input_info.material = material;
    meshes.add( &input_info );

    return driver->createModelFromMemory( "input_saturate", meshes );
}
static StormGraph::IModel* model_ui_modal_shadow( StormGraph::IGraphicsDriver* driver, StormGraph::IMaterial* material, float width, float height )
{
    li::List<StormGraph::MeshCreationInfo*> meshes;

    float shadow_coords[] =
    {
        0.0f, 0.0f, 0.0f,
        0.0f, height, 0.0f,
        width, 0.0f, 0.0f,
        width, 0.0f, 0.0f,
        0.0f, height, 0.0f,
        width, height, 0.0f,
    };

    StormGraph::MeshCreationInfo shadow_info;
    shadow_info.format = StormGraph::MeshFormat::triangleList;
    shadow_info.layout = StormGraph::MeshLayout::linear;
    shadow_info.numVertices = 6;
    shadow_info.numIndices = 0;
    shadow_info.coords = shadow_coords;
    shadow_info.normals = 0;
    shadow_info.uvs = 0;
    shadow_info.indices = 0;
    shadow_info.material = material;
    meshes.add( &shadow_info );

    return driver->createModelFromMemory( "ui_modal_shadow", meshes );
}
static StormGraph::IModel* model_window( StormGraph::IGraphicsDriver* driver, StormGraph::IMaterial** materials, float width, float height )
{
    li::List<StormGraph::MeshCreationInfo*> meshes;

    float panel_coords[] =
    {
        8.0f, 8.0f, 0.0f,
        8.0f, height - 8.0f, 0.0f,
        width - 8.0f, 8.0f, 0.0f,
        width - 8.0f, 8.0f, 0.0f,
        8.0f, height - 8.0f, 0.0f,
        width - 8.0f, height - 8.0f, 0.0f,
    };

    float panel_uvs[] =
    {
        0.0f, 0.0f,
        0.0f, ( height - 16.0f ) / 64.0f,
        ( width - 16.0f ) / 64.0f, 0.0f,
        ( width - 16.0f ) / 64.0f, 0.0f,
        0.0f, ( height - 16.0f ) / 64.0f,
        ( width - 16.0f ) / 64.0f, ( height - 16.0f ) / 64.0f,
    };

    StormGraph::MeshCreationInfo panel_info;
    panel_info.format = StormGraph::MeshFormat::triangleList;
    panel_info.layout = StormGraph::MeshLayout::linear;
    panel_info.numVertices = 6;
    panel_info.numIndices = 0;
    panel_info.coords = panel_coords;
    panel_info.normals = 0;
    panel_info.uvs = panel_uvs;
    panel_info.indices = 0;
    panel_info.material = materials[0]->reference();
    meshes.add( &panel_info );

    float corners_coords[] =
    {
        -24.0f, -24.0f, 0.0f,
        -24.0f, 8.0f, 0.0f,
        8.0f, -24.0f, 0.0f,
        8.0f, -24.0f, 0.0f,
        -24.0f, 8.0f, 0.0f,
        8.0f, 8.0f, 0.0f,
        width - 8.0f, -24.0f, 0.0f,
        width - 8.0f, 8.0f, 0.0f,
        width + 24.0f, -24.0f, 0.0f,
        width + 24.0f, -24.0f, 0.0f,
        width - 8.0f, 8.0f, 0.0f,
        width + 24.0f, 8.0f, 0.0f,
        -24.0f, height - 8.0f, 0.0f,
        -24.0f, height + 24.0f, 0.0f,
        8.0f, height - 8.0f, 0.0f,
        8.0f, height - 8.0f, 0.0f,
        -24.0f, height + 24.0f, 0.0f,
        8.0f, height + 24.0f, 0.0f,
        width - 8.0f, height - 8.0f, 0.0f,
        width - 8.0f, height + 24.0f, 0.0f,
        width + 24.0f, height - 8.0f, 0.0f,
        width + 24.0f, height - 8.0f, 0.0f,
        width - 8.0f, height + 24.0f, 0.0f,
        width + 24.0f, height + 24.0f, 0.0f,
    };

    float corners_uvs[] =
    {
        0.0f, 0.0f,
        0.0f, 0.5f,
        0.5f, 0.0f,
        0.5f, 0.0f,
        0.0f, 0.5f,
        0.5f, 0.5f,
        0.5f, 0.0f,
        0.5f, 0.5f,
        1.0f, 0.0f,
        1.0f, 0.0f,
        0.5f, 0.5f,
        1.0f, 0.5f,
        0.0f, 0.5f,
        0.0f, 1.0f,
        0.5f, 0.5f,
        0.5f, 0.5f,
        0.0f, 1.0f,
        0.5f, 1.0f,
        0.5f, 0.5f,
        0.5f, 1.0f,
        1.0f, 0.5f,
        1.0f, 0.5f,
        0.5f, 1.0f,
        1.0f, 1.0f,
    };

    StormGraph::MeshCreationInfo corners_info;
    corners_info.format = StormGraph::MeshFormat::triangleList;
    corners_info.layout = StormGraph::MeshLayout::linear;
    corners_info.numVertices = 24;
    corners_info.numIndices = 0;
    corners_info.coords = corners_coords;
    corners_info.normals = 0;
    corners_info.uvs = corners_uvs;
    corners_info.indices = 0;
    corners_info.material = materials[1]->reference();
    meshes.add( &corners_info );

    float top_bottom_coords[] =
    {
        8.0f, -24.0f, 0.0f,
        8.0f, 8.0f, 0.0f,
        width - 8.0f, -24.0f, 0.0f,
        width - 8.0f, -24.0f, 0.0f,
        8.0f, 8.0f, 0.0f,
        width - 8.0f, 8.0f, 0.0f,
        8.0f, height - 8.0f, 0.0f,
        8.0f, height + 24.0f, 0.0f,
        width - 8.0f, height - 8.0f, 0.0f,
        width - 8.0f, height - 8.0f, 0.0f,
        8.0f, height + 24.0f, 0.0f,
        width - 8.0f, height + 24.0f, 0.0f,
    };

    float top_bottom_uvs[] =
    {
        0.0f, 0.0f,
        0.0f, 0.5f,
        ( width - 16.0f ) / 64.0f, 0.0f,
        ( width - 16.0f ) / 64.0f, 0.0f,
        0.0f, 0.5f,
        ( width - 16.0f ) / 64.0f, 0.5f,
        0.0f, 0.5f,
        0.0f, 1.0f,
        ( width - 16.0f ) / 64.0f, 0.5f,
        ( width - 16.0f ) / 64.0f, 0.5f,
        0.0f, 1.0f,
        ( width - 16.0f ) / 64.0f, 1.0f,
    };

    StormGraph::MeshCreationInfo top_bottom_info;
    top_bottom_info.format = StormGraph::MeshFormat::triangleList;
    top_bottom_info.layout = StormGraph::MeshLayout::linear;
    top_bottom_info.numVertices = 12;
    top_bottom_info.numIndices = 0;
    top_bottom_info.coords = top_bottom_coords;
    top_bottom_info.normals = 0;
    top_bottom_info.uvs = top_bottom_uvs;
    top_bottom_info.indices = 0;
    top_bottom_info.material = materials[2]->reference();
    meshes.add( &top_bottom_info );

    float left_right_coords[] =
    {
        -24.0f, 8.0f, 0.0f,
        -24.0f, height - 8.0f, 0.0f,
        8.0f, 8.0f, 0.0f,
        8.0f, 8.0f, 0.0f,
        -24.0f, height - 8.0f, 0.0f,
        8.0f, height - 8.0f, 0.0f,
        width - 8.0f, 8.0f, 0.0f,
        width - 8.0f, height - 8.0f, 0.0f,
        width + 24.0f, 8.0f, 0.0f,
        width + 24.0f, 8.0f, 0.0f,
        width - 8.0f, height - 8.0f, 0.0f,
        width + 24.0f, height - 8.0f, 0.0f,
    };

    float left_right_uvs[] =
    {
        0.0f, 0.0f,
        0.0f, ( height - 16.0f ) / 64.0f,
        0.5f, 0.0f,
        0.5f, 0.0f,
        0.0f, ( height - 16.0f ) / 64.0f,
        0.5f, ( height - 16.0f ) / 64.0f,
        0.5f, 0.0f,
        0.5f, ( height - 16.0f ) / 64.0f,
        1.0f, 0.0f,
        1.0f, 0.0f,
        0.5f, ( height - 16.0f ) / 64.0f,
        1.0f, ( height - 16.0f ) / 64.0f,
    };

    StormGraph::MeshCreationInfo left_right_info;
    left_right_info.format = StormGraph::MeshFormat::triangleList;
    left_right_info.layout = StormGraph::MeshLayout::linear;
    left_right_info.numVertices = 12;
    left_right_info.numIndices = 0;
    left_right_info.coords = left_right_coords;
    left_right_info.normals = 0;
    left_right_info.uvs = left_right_uvs;
    left_right_info.indices = 0;
    left_right_info.material = materials[3]->reference();
    meshes.add( &left_right_info );

    return driver->createModelFromMemory( "window", meshes );
}
