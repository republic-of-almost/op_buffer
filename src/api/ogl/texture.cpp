#ifdef OP_BUFFER_API_OGL


#include "texture.hpp"
#include "details/ogl_common.hpp"
#include "details/error_code_to_text.hpp"
#include "details/enum_to_string.hpp"
#include "details/op_common.hpp"
#include "details/op_to_ogl.hpp"
#include "descs/descs.hpp"


namespace {


} // ns


namespace op {
namespace ogl {


void
texture_create(context_data *context, void *data)
{
  // -- Param Check -- //
  assert(context);
  assert(data);

  // -- Get Cmd Data -- //
  command::cmd_texture_create *cmd(
    reinterpret_cast<command::cmd_texture_create*>(data)
  );

  // -- Get Texture Data -- //
  ogl::texture_desc *internal_desc(
    &context->texture_descs[cmd->texture_id]
  );
  assert(internal_desc);

  // -- Generate Texture -- //
  GLuint texture = 0;

  glGenTextures(1, &texture);

  if(texture)
  {
    GLsizei height              = cmd->desc->height; // non const might get modified.
    const GLsizei width         = cmd->desc->width;
    const GLsizei depth         = cmd->desc->depth;
    const GLint internal_format = op_pixel_format_to_internal_format(cmd->desc->format);
    const GLenum format         = op_pixel_format_to_format(cmd->desc->format);
    const GLenum type           = op_pixel_format_to_type(cmd->desc->format);
    const GLenum dimention      = op_dimention_to_texture(cmd->desc->dimention);

    switch(cmd->desc->dimention)
    {
      #ifdef OGL_HAS_TEXTURE_1D
      case(opDimention_ONE):
      {
        glBindTexture(dimention, texture);
        glTexImage1D(dimention,
                     0,
                     internal_format,
                     width,
                     0,
                     format,
                     type,
                     cmd->data);
        break;
      }
      #else
      // Simulated 1D texture.
      case(opDimention_TWO):
      {
        height = 1;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     internal_format,
                     width,
                     height,
                     0,
                     format,
                     type,
                     cmd->data);

        break;
      }
      #endif

      case(opDimention_TWO):
      {
        glBindTexture(dimention, texture);
        glTexImage2D(dimention,
                     0,
                     internal_format,
                     width,
                     height,
                     0,
                     format,
                     type,
                     cmd->data);

        break;
      }

      #ifdef OGL_HAS_TEXTURE_3D
      case(opDimention_THREE):
      {
        glBindTexture(dimention, texture);
        glTexImage3D(dimention,
                     0,
                     internal_format,
                     width,
                     height,
                     depth,
                     0,
                     format,
                     type,
                     cmd->data);

        break;
      }
      #endif

      // Gen Mips //
      if(cmd->desc->mips)
      {
        glGenerateMipmap(dimention);
      }
    }

    // Update internal format
    internal_desc->format          = format;
    internal_desc->internal_format = internal_format;
    internal_desc->type            = type;
    internal_desc->texture_id      = texture;
    internal_desc->width           = width;
    internal_desc->height          = height;
    internal_desc->depth           = depth;
    internal_desc->dimention       = dimention;

    // -- Extra Logging -- //
    #ifdef OP_BUFFER_LOG_INFO
    context->log("[");
    context->log("  Texture Created");
    context->log("  --");
    context->log("  Dimentions: %d x %d x %d", width, height, depth);
    context->log("  Dimention: %s", get_texture_dimention_name(GL_TEXTURE_2D));
    context->log("  Internal Format: %s", get_pixel_format_name(internal_format));
    context->log("  Format: %s", get_pixel_format_component_name(format));
    context->log("  Type: %s", get_type_name(type));
    context->log("  GL ID: %d", texture);
    context->log("]");
    #endif
  }
  //
  #ifdef OP_BUFFER_LOG_INFO
  else
  {
    context->log("OP Error - Texture Did not load");
  }
  #endif

  // -- Update In/Out Desc -- //
  cmd->desc->status = texture ? opStatus_VALID : opStatus_INVALID;

  // -- Extra Check -- //
  #ifdef OP_BUFFER_API_OGL_EXTRA_CHECKS
  const GLenum err_code = glGetError();

  if(err_code)
  {
    context->log(
      "GL Error - Texture Create %d : %s",
      err_code,
      get_error_msg(err_code)
    );
  }
  #endif
}


void
texture_update(context_data *context, void *data)
{
  // -- Param Check -- //
  assert(context);
  assert(data);

  // -- Get Cmd Data -- //
  command::cmd_texture_update *cmd(
    reinterpret_cast<command::cmd_texture_update*>(data)
  );

  // -- Get Texture Data -- //
  ogl::texture_desc *internal_desc(
    &context->texture_descs[cmd->texture_id]
  );
  assert(internal_desc);

  // -- Pixel Alignment -- //
  if(internal_desc->format == GL_RED)
  {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  }

  // -- Update Resource -- //
  switch(internal_desc->dimention)
  {
    #ifdef OGL_HAS_TEXTURE_1D
    case(GL_TEXTURE_1D):
    {
      glBindTexture(GL_TEXTURE_1D, internal_desc->texture_id);
      glTexSubImage1D(GL_TEXTURE_1D,
                      0,
                      cmd->offset_x,
                      cmd->width,
                      internal_desc->format,
                      internal_desc->type,
                      cmd->data);
      break;
    }
    #endif

    case(GL_TEXTURE_2D):
    {
      glBindTexture(GL_TEXTURE_2D, internal_desc->texture_id);
      glTexSubImage2D(GL_TEXTURE_2D,
                      0,
                      cmd->offset_x,
                      cmd->offset_y,
                      cmd->width,
                      cmd->height,
                      internal_desc->format,
                      internal_desc->type,
                      cmd->data);
      break;
    }

    #ifdef OGL_HAS_TEXTURE_3D
    case(GL_TEXTURE_3D):
    {
      glBindTexture(GL_TEXTURE_3D, internal_desc->texture_id);
      glTexSubImage3D(GL_TEXTURE_3D,
                      0,
                      cmd->offset_x,
                      cmd->offset_y,
                      cmd->offset_z,
                      cmd->width,
                      cmd->height,
                      cmd->depth,
                      internal_desc->format,
                      internal_desc->type,
                      cmd->data);
      break;
    }
    #endif
  }

  // -- Gen Mips -- //
  if(internal_desc->mips)
  {
    glGenerateMipmap(internal_desc->dimention);
  }

  // Put packing back to what it was. //
  if(internal_desc->format == GL_RED)
  {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  }

  // -- Extra Check -- //
  #ifdef OP_BUFFER_API_OGL_EXTRA_CHECKS
  const GLenum err_code = glGetError();

  if(err_code)
  {
    context->log(
      "GL Error - Texture Update %d : %s",
      err_code,
      get_error_msg(err_code)
    );
  }
  #endif
}


void
texture_destroy(context_data *context, void *data)
{
}


void
texture_bind(context_data *context, void *data)
{
  // Param Check //
  assert(context);
  assert(data);

  // -- Get Cmd Data -- //
  command::cmd_texture_bind *cmd(
    reinterpret_cast<command::cmd_texture_bind*>(data)
  );

  // -- Last Bind Cache -- //
  if(context->texture_currently_bound &&
     context->texture_last_bind == cmd->texture_id)
  {
    return;
  }

  context->texture_currently_bound = true;
  context->texture_last_bind = cmd->texture_id;

  // -- Get Index Data -- //
  ogl::texture_desc *internal_desc(
    &context->texture_descs[cmd->texture_id]
  );
}


} // ns
} // ns


#endif // api guard
