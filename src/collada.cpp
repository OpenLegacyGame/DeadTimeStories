#include "collada.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <pugixml.hpp>
#include <sstream>
#include <stdexcept>

namespace deadtime {
namespace {
using Matrix = std::array<float, 16>;
Matrix identity() {
    return {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
}
Matrix multiply(const Matrix& a, const Matrix& b) {
    Matrix out{};
    for (unsigned r = 0; r < 4; ++r)
        for (unsigned c = 0; c < 4; ++c)
            for (unsigned k = 0; k < 4; ++k)
                out[r * 4 + c] += a[r * 4 + k] * b[k * 4 + c];
    return out;
}
ModelVector transform(const Matrix& m, ModelVector v) {
    return {m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3], m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7],
            m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11]};
}
Matrix rotation(ModelVector axis, float degrees) {
    const float length = std::sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
    if (length == 0)
        throw std::runtime_error("Zero Collada rotation axis");
    const float x = axis.x / length, y = axis.y / length, z = axis.z / length;
    const float angle = degrees * 3.14159265358979323846f / 180, c = std::cos(angle), s = std::sin(angle),
                t = 1 - c;
    return {t * x * x + c,
            t * x * y - s * z,
            t * x * z + s * y,
            0,
            t * x * y + s * z,
            t * y * y + c,
            t * y * z - s * x,
            0,
            t * x * z - s * y,
            t * y * z + s * x,
            t * z * z + c,
            0,
            0,
            0,
            0,
            1};
}
ModelVector convert(ModelVector v) {
    return {v.x, v.z, v.y};
}
template <typename T> std::vector<T> numbers(const char* text) {
    std::istringstream stream(text);
    std::vector<T> result;
    T value;
    while (stream >> value) {
        if (!std::isfinite(double(value)))
            throw std::runtime_error("Non-finite Collada number");
        result.push_back(value);
    }
    if (!stream.eof())
        throw std::runtime_error("Invalid Collada number");
    return result;
}
std::string fragment(const char* value) {
    std::string result(value);
    if (result.empty() || result[0] != '#')
        throw std::runtime_error("External Collada reference: " + result);
    return result.substr(1);
}
std::string unescape(std::string text) {
    auto hex = [](char c) -> int {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        return -1;
    };
    std::string out;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '%') {
            if (i + 2 >= text.size() || hex(text[i + 1]) < 0 || hex(text[i + 2]) < 0)
                throw std::runtime_error("Invalid Collada URL escape");
            const auto c = char(hex(text[i + 1]) * 16 + hex(text[i + 2]));
            if (!c)
                throw std::runtime_error("NUL in Collada URL");
            out += c;
            i += 2;
        } else
            out += text[i];
    }
    return out;
}
struct Source {
    std::vector<float> data;
    size_t stride = 1, offset = 0, count = 0;
    std::vector<float> at(size_t index, size_t components) const {
        if (index >= count || components > stride || offset + index * stride + components > data.size())
            throw std::runtime_error("Collada accessor out of range");
        return {data.begin() + offset + index * stride, data.begin() + offset + index * stride + components};
    }
};
// SDL clamps texture coordinates. Split triangles at integer UV boundaries to
// preserve the native shader's WRAP sampler, including the original edge UVs.
std::vector<ModelVertex> wrapTriangles(const std::vector<ModelVertex>& input) {
    std::vector<ModelVertex> out;
    for (size_t i = 0; i < input.size(); i += 3) {
        float minU = input[i].u, maxU = minU, minV = input[i].v, maxV = minV;
        for (size_t j = 1; j < 3; ++j) {
            minU = std::min(minU, input[i + j].u);
            maxU = std::max(maxU, input[i + j].u);
            minV = std::min(minV, input[i + j].v);
            maxV = std::max(maxV, input[i + j].v);
        }
        if (minU < -64 || maxU > 64 || minV < -64 || maxV > 64 || maxU - minU > 16 || maxV - minV > 16)
            throw std::runtime_error("Oversized Collada UV range");
        const int left = int(std::floor(minU)), right = std::max(left, int(std::ceil(maxU)) - 1),
                  top = int(std::floor(minV)), bottom = std::max(top, int(std::ceil(maxV)) - 1);
        for (int x = left; x <= right; ++x)
            for (int y = top; y <= bottom; ++y) {
                std::vector<ModelVertex> polygon{input[i], input[i + 1], input[i + 2]};
                auto clip = [&](bool horizontal, float boundary, bool greater) {
                    std::vector<ModelVertex> clipped;
                    if (polygon.empty())
                        return;
                    auto previous = polygon.back();
                    for (auto current : polygon) {
                        const float a = horizontal ? previous.u : previous.v,
                                    b = horizontal ? current.u : current.v;
                        const bool aInside = greater ? a >= boundary : a <= boundary,
                                   bInside = greater ? b >= boundary : b <= boundary;
                        if (aInside != bInside) {
                            const float t = (boundary - a) / (b - a);
                            ModelVertex v{
                                {previous.position.x + t * (current.position.x - previous.position.x),
                                 previous.position.y + t * (current.position.y - previous.position.y),
                                 previous.position.z + t * (current.position.z - previous.position.z)},
                                previous.u + t * (current.u - previous.u),
                                previous.v + t * (current.v - previous.v)};
                            clipped.push_back(v);
                        }
                        if (bInside)
                            clipped.push_back(current);
                        previous = current;
                    }
                    polygon = std::move(clipped);
                };
                clip(true, float(x), true);
                clip(true, float(x + 1), false);
                clip(false, float(y), true);
                clip(false, float(y + 1), false);
                for (auto& v : polygon) {
                    v.u = std::clamp(v.u - x, 0.f, 1.f);
                    v.v = std::clamp(v.v - y, 0.f, 1.f);
                }
                for (size_t j = 1; j + 1 < polygon.size(); ++j)
                    out.insert(out.end(), {polygon[0], polygon[j], polygon[j + 1]});
            }
    }
    return out;
}
} // namespace
ModelTransform modelIdentity() {
    return identity();
}
ModelVector modelPoint(const ModelTransform& matrix, ModelVector point) {
    return transform(matrix, point);
}
std::vector<float> ColladaScene::Track::sample(double time) const {
    if (time <= times.front())
        return values.front();
    if (time >= times.back())
        return values.back();
    const size_t next = std::upper_bound(times.begin(), times.end(), time) - times.begin(),
                 previous = next - 1;
    const float t = float((time - times[previous]) / (times[next] - times[previous]));
    auto out = values[previous];
    for (size_t i = 0; i < out.size(); ++i)
        out[i] += t * (values[next][i] - out[i]);
    return out;
}
ColladaScene::ColladaScene(const Assets& assets, const std::string& path) {
    pugi::xml_document document;
    const auto bytes = assets.read(path);
    if (!document.load_buffer(bytes.data(), bytes.size()))
        throw std::runtime_error("Invalid Collada XML: " + path);
    const auto root = document.child("COLLADA");
    if (!root || std::string(root.child("asset").child("up_axis").text().as_string()) != "Z_UP")
        throw std::runtime_error("Unsupported Collada axes: " + path);
    std::map<std::string, pugi::xml_node> ids;
    std::function<void(pugi::xml_node)> index = [&](auto node) {
        if (auto id = node.attribute("id"))
            if (!ids.emplace(id.value(), node).second)
                throw std::runtime_error("Duplicate Collada ID");
        for (auto child : node.children())
            index(child);
    };
    index(root);
    auto lookup = [&](const std::string& id) {
        auto i = ids.find(id);
        if (i == ids.end())
            throw std::runtime_error("Missing Collada ID: " + id);
        return i->second;
    };
    auto source = [&](const std::string& id) {
        auto node = lookup(id), accessor = node.child("technique_common").child("accessor");
        Source out;
        out.data = numbers<float>(lookup(fragment(accessor.attribute("source").value())).text().as_string());
        out.stride = accessor.attribute("stride").as_uint(1);
        out.offset = accessor.attribute("offset").as_uint();
        out.count = accessor.attribute("count").as_uint();
        if (!out.stride || out.count > out.data.size() / out.stride ||
            out.offset > out.data.size() - out.count * out.stride)
            throw std::runtime_error("Invalid Collada accessor");
        return out;
    };
    auto material = [&](const std::string& id) {
        ModelMesh out;
        auto effectInstance = lookup(id).child("instance_effect"),
             effect = lookup(fragment(effectInstance.attribute("url").value()));
        std::string shader;
        for (auto technique : effect.child("extra").children("technique"))
            if (std::string(technique.attribute("profile").value()) == "NV_import")
                shader =
                    resolve(directory(path), unescape(technique.child("import").attribute("url").value()));
        if (shader == "materials/additive_texture_a.fx")
            out.additive = true;
        else if (shader != "materials/pure_texture_a.fx")
            throw std::runtime_error("Unsupported Collada shader: " + shader);
        std::string image;
        for (auto param : effectInstance.children("setparam"))
            if (std::string(param.attribute("ref").value()) == "textureDiffuse")
                image = param.child("surface").child("init_from").text().as_string();
        out.texture = resolve(directory(path), unescape(lookup(image).child("init_from").text().as_string()));
        return out;
    };
    std::map<std::string, size_t> nodeIndices;
    std::function<void(pugi::xml_node, int)> node = [&](auto element, int parent) {
        if (nodes_.size() > 10000)
            throw std::runtime_error("Oversized Collada node tree");
        const auto current = nodes_.size();
        nodes_.push_back({});
        Node value;
        value.parent = parent;
        value.id = element.attribute("id").value();
        value.name = element.attribute("name").as_string(value.id.c_str());
        nodeIndices[value.id] = current;
        if (auto translation = element.child("translate")) {
            const auto v = numbers<float>(translation.text().as_string());
            if (v.size() != 3)
                throw std::runtime_error("Invalid Collada translation");
            value.translation = {v[0], v[1], v[2]};
            value.translationSid = translation.attribute("sid").value();
        }
        for (auto r : element.children("rotate")) {
            auto v = numbers<float>(r.text().as_string());
            if (v.size() != 4)
                throw std::runtime_error("Invalid Collada rotation");
            value.rotations.push_back({{v[0], v[1], v[2]}, v[3], r.attribute("sid").value()});
        }
        // Native 5B0760 reads translations/rotations, but does not import scale
        // elements. Native 5ADB20 also leaves the unit multiplier at one.
        if (element.child("matrix"))
            throw std::runtime_error("Collada matrix import not implemented");
        for (auto instance : element.children("instance_geometry")) {
            std::map<std::string, std::string> bindings;
            for (auto binding :
                 instance.child("bind_material").child("technique_common").children("instance_material"))
                bindings[binding.attribute("symbol").value()] = fragment(binding.attribute("target").value());
            const auto mesh = lookup(fragment(instance.attribute("url").value())).child("mesh");
            for (auto triangles : mesh.children("triangles")) {
                auto binding = bindings.find(triangles.attribute("material").value());
                if (binding == bindings.end())
                    throw std::runtime_error("Unbound Collada material");
                auto output = material(binding->second);
                std::string positionId, uvId;
                size_t positionOffset = 0, uvOffset = 0, stride = 0;
                unsigned uvSet = std::numeric_limits<unsigned>::max();
                for (auto input : triangles.children("input")) {
                    const auto offset = input.attribute("offset").as_uint();
                    stride = std::max(stride, size_t(offset + 1));
                    const std::string semantic = input.attribute("semantic").value();
                    if (semantic == "VERTEX") {
                        for (auto vertex :
                             lookup(fragment(input.attribute("source").value())).children("input"))
                            if (std::string(vertex.attribute("semantic").value()) == "POSITION") {
                                positionId = fragment(vertex.attribute("source").value());
                                positionOffset = offset;
                            }
                    } else if (semantic == "TEXCOORD" && input.attribute("set").as_uint() < uvSet) {
                        uvSet = input.attribute("set").as_uint();
                        uvId = fragment(input.attribute("source").value());
                        uvOffset = offset;
                    }
                }
                if (positionId.empty() || uvId.empty() || !stride)
                    throw std::runtime_error("Missing Collada vertex input");
                const auto positions = source(positionId), uv = source(uvId);
                const auto indices = numbers<size_t>(triangles.child("p").text().as_string());
                const size_t count = triangles.attribute("count").as_uint();
                if (count > 1000000 || indices.size() != count * 3 * stride)
                    throw std::runtime_error("Invalid Collada triangle count");
                for (size_t i = 0; i < indices.size(); i += stride) {
                    auto p = positions.at(indices[i + positionOffset], 3),
                         t = uv.at(indices[i + uvOffset], 2);
                    output.vertices.push_back({{p[0], p[1], p[2]}, t[0], 1 - t[1]});
                }
                output.vertices = wrapTriangles(output.vertices);
                value.meshes.push_back(std::move(output));
            }
        }
        nodes_[current] = std::move(value);
        for (auto child : element.children("node"))
            node(child, int(current));
    };
    const auto visual =
        lookup(fragment(root.child("scene").child("instance_visual_scene").attribute("url").value()));
    for (auto child : visual.children("node"))
        node(child, -1);
    std::function<void(pugi::xml_node)> animation = [&](auto element) {
        for (auto channel : element.children("channel")) {
            const std::string target = channel.attribute("target").value();
            const auto slash = target.find('/');
            auto found = nodeIndices.find(target.substr(0, slash));
            if (found == nodeIndices.end() || slash == std::string::npos)
                throw std::runtime_error("Unknown Collada animation node");
            const auto sampler = lookup(fragment(channel.attribute("source").value()));
            std::string input, output, interpolation;
            for (auto i : sampler.children("input")) {
                const std::string semantic = i.attribute("semantic").value(),
                                  id = fragment(i.attribute("source").value());
                if (semantic == "INPUT")
                    input = id;
                else if (semantic == "OUTPUT")
                    output = id;
                else if (semantic == "INTERPOLATION")
                    interpolation = id;
            }
            auto times = source(input), values = source(output);
            if (times.count == 0 || times.count != values.count || times.stride != 1)
                throw std::runtime_error("Invalid Collada key count");
            if (!interpolation.empty()) {
                std::istringstream modes(lookup(interpolation).child("Name_array").text().as_string());
                std::string mode;
                while (modes >> mode)
                    if (mode != "LINEAR")
                        throw std::runtime_error("Unsupported Collada interpolation: " + mode);
            }
            Track track;
            for (size_t i = 0; i < times.count; ++i) {
                const auto t = times.at(i, 1).front();
                if (t < 0 || (i && t <= track.times.back()))
                    throw std::runtime_error("Unordered Collada keys");
                track.times.push_back(t);
                track.values.push_back(values.at(i, values.stride));
            }
            auto& destination = nodes_[found->second];
            const auto property = target.substr(slash + 1);
            const bool translation = property == destination.translationSid ||
                                     property == destination.translationSid + ".X" ||
                                     property == destination.translationSid + ".Y" ||
                                     property == destination.translationSid + ".Z";
            const bool angle =
                std::any_of(destination.rotations.begin(), destination.rotations.end(),
                            [&](const auto& r) { return !r.sid.empty() && property == r.sid + ".ANGLE"; });
            if (!translation && !angle)
                throw std::runtime_error("Unsupported Collada animation target: " + target);
            if (values.stride != (property == destination.translationSid ? 3u : 1u))
                throw std::runtime_error("Invalid Collada animation components");
            destination.duration = std::max(destination.duration, track.times.back());
            duration_ = std::max(duration_, destination.duration);
            if (!destination.tracks.emplace(property, std::move(track)).second)
                throw std::runtime_error("Duplicate Collada animation target");
        }
        for (auto child : element.children("animation"))
            animation(child);
    };
    animation(root.child("library_animations"));
}
ModelFrame ColladaScene::frame(double seconds, bool looped) const {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid Collada playback time");
    ModelFrame result;
    std::vector<Matrix> matrices;
    matrices.reserve(nodes_.size());
    for (const auto& node : nodes_) {
        const auto time = looped && node.duration > 0 ? std::fmod(seconds, node.duration) : seconds;
        auto translation = node.translation;
        auto rotations = node.rotations;
        for (const auto& [property, track] : node.tracks) {
            const auto value = track.sample(time);
            if (property == node.translationSid)
                translation = {value[0], value[1], value[2]};
            else if (property == node.translationSid + ".X")
                translation.x = value[0];
            else if (property == node.translationSid + ".Y")
                translation.y = value[0];
            else if (property == node.translationSid + ".Z")
                translation.z = value[0];
            else
                for (auto& r : rotations)
                    if (property == r.sid + ".ANGLE")
                        r.degrees = value[0];
        }
        auto local = identity();
        local[3] = translation.x;
        local[7] = translation.y;
        local[11] = translation.z;
        const bool animatedRotation =
            std::any_of(node.tracks.begin(), node.tracks.end(),
                        [](const auto& track) { return track.first.ends_with(".ANGLE"); });
        if (animatedRotation) {
            // The native importer rebuilds animated rotations with its
            // AnimationEulerSeparated controller, after axis conversion and
            // angle negation. This differs from multiplying the XML rotate
            // elements in place when both Y and Z rotate.
            ModelVector angles;
            for (const auto& r : rotations) {
                const auto axis = convert(r.axis);
                if (std::abs(axis.x) == 1 && axis.y == 0 && axis.z == 0)
                    angles.x = -axis.x * r.degrees;
                else if (std::abs(axis.y) == 1 && axis.x == 0 && axis.z == 0)
                    angles.y = -axis.y * r.degrees;
                else if (std::abs(axis.z) == 1 && axis.x == 0 && axis.y == 0)
                    angles.z = -axis.z * r.degrees;
                else
                    throw std::runtime_error("Unsupported animated Collada rotation axis");
            }
            const Matrix swap{1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1};
            const auto native =
                multiply(rotation({0, 0, 1}, angles.z),
                         multiply(rotation({0, 1, 0}, angles.y), rotation({1, 0, 0}, angles.x)));
            local = multiply(local, multiply(swap, multiply(native, swap)));
        } else
            for (const auto& r : rotations)
                local = multiply(local, rotation(r.axis, r.degrees));
        const auto world = node.parent < 0 ? local : multiply(matrices.at(size_t(node.parent)), local);
        matrices.push_back(world);
        result.nodes[node.name] = convert(transform(world, {}));
        result.nodeOrder.push_back(node.name);
        const Matrix axisSwap{1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1};
        result.transforms[node.name] = multiply(axisSwap, multiply(world, axisSwap));
        for (const auto& mesh : node.meshes) {
            auto output = mesh;
            for (auto& vertex : output.vertices)
                vertex.position = convert(transform(world, vertex.position));
            result.meshes.push_back(std::move(output));
        }
    }
    return result;
}
} // namespace deadtime
