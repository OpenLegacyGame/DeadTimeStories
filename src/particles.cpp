#include "particles.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <pugixml.hpp>
#include <stdexcept>

namespace deadtime {
namespace {
ModelVector add(ModelVector a, ModelVector b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}
ModelVector scale(ModelVector a, float b) {
    return {a.x * b, a.y * b, a.z * b};
}
double number(pugi::xml_attribute a, double fallback = 0) {
    const double value = a ? a.as_double() : fallback;
    if (!std::isfinite(value))
        throw std::runtime_error("Non-finite particle configuration value");
    return value;
}
ModelVector point(pugi::xml_node node) {
    return {float(number(node.attribute("x"))), float(number(node.attribute("y"))),
            float(number(node.attribute("z")))};
}
float bounded(float value) {
    if (std::isnan(value))
        return 1;
    return std::clamp(value, 0.f, 1.f);
}
} // namespace
float ParticleCurve::sample(float time) const {
    if (keys.empty())
        return 0;
    if (std::isnan(time))
        return 0;
    if (!std::isfinite(time)) {
        if (time < 0)
            return keys.size() > 1 && loopLeft ? 0 : keys.front().in;
        return keys.size() > 1 && loopRight ? 0 : keys.back().out;
    }
    const auto& first = keys.front();
    const auto& last = keys.back();
    const float length = last.time - first.time;
    if (time < first.time) {
        if (keys.size() == 1 || !loopLeft || length <= 0)
            return first.in;
        time = last.time - std::fmod(first.time - time, length);
    }
    if (time >= last.time) {
        if (keys.size() == 1 || !loopRight || length <= 0)
            return last.out;
        time = first.time + std::fmod(time - last.time, length);
    }
    for (size_t i = 0; i + 1 < keys.size(); ++i) {
        const auto& a = keys[i];
        const auto& b = keys[i + 1];
        if (time < a.time || time > b.time)
            continue;
        const float span = b.time - a.time;
        if (span <= std::numeric_limits<float>::min())
            return a.out;
        const float t = (time - a.time) / span;
        if (a.type == 0)
            return a.out;
        if (a.type == 1)
            return float(double(t) * b.in + (1. - t) * a.out);
        if (a.type == 3 || a.type == 4) {
            const float left = span * a.outY / a.outX, right = span * b.inY / b.inX, t2 = t * t, t3 = t * t2;
            return float((t3 - 2. * t2 + t) * left + (2. * t3 - 3. * t2 + 1) * a.out +
                         (3. * t2 - 2. * t3) * b.in + (t3 - t2) * right);
        }
        if (a.type == 2) {
            // Original track type 2 is two joined quadratic Bezier segments.
            const float x1 = a.time + a.outX * span, y1 = a.out + a.outY * span, x2 = b.time + b.inX * span,
                        y2 = b.in + b.inY * span, mx = (x1 + x2) * .5f, my = (y1 + y2) * .5f;
            const bool left = time < mx;
            const double x0 = left ? a.time : mx, xc = left ? x1 : x2, xe = left ? mx : b.time;
            const double y0 = left ? a.out : my, yc = left ? y1 : y2, ye = left ? my : b.in;
            const double aa = x0 - 2 * xc + xe, bb = 2 * (xc - x0), cc = x0 - time;
            double u = 0;
            if (std::abs(aa) < 1e-12)
                u = std::abs(bb) < 1e-12 ? 0 : -cc / bb;
            else {
                const double disc = bb * bb - 4 * aa * cc;
                if (disc < 0)
                    return 0;
                const double root = std::sqrt(disc), u1 = (-bb + root) / (2 * aa),
                             u2 = (-bb - root) / (2 * aa);
                u = std::abs(.5 - u1) < std::abs(.5 - u2) ? u1 : u2;
            }
            return float((1 - u) * (1 - u) * y0 + 2 * (1 - u) * u * yc + u * u * ye);
        }
        throw std::runtime_error("Unsupported particle curve key");
    }
    return 0;
}
double ParticleLayout::Distribution::scalar(ParticleRandom& random) const {
    if (mode == Fixed)
        return from[0];
    const float t = random.unit();
    return double(t) * to[0] + (1. - t) * from[0];
}
ModelVector ParticleLayout::Distribution::vector(ParticleRandom& random) const {
    if (mode == Fixed)
        return {float(from[0]), float(from[1]), float(from[2])};
    const float z = random.unit(), y = mode == Box ? random.unit() : z, x = mode == Box ? random.unit() : z;
    return {float(x * to[0] + (1. - x) * from[0]), float(y * to[1] + (1. - y) * from[1]),
            float(z * to[2] + (1. - z) * from[2])};
}
ParticleLayout::ParticleLayout(const Assets& assets, const std::string& path) {
    pugi::xml_document doc;
    const auto bytes = assets.read(path);
    if (!doc.load_buffer(bytes.data(), bytes.size()) || !doc.child("particles_layout"))
        throw std::runtime_error("Invalid particle XML: " + path);
    const auto root = doc.child("particles_layout");
    worldEmitting = root.child("world_emitting").attribute("value").as_bool(true);
    auto distribution = [&](pugi::xml_node node, bool vector, bool wide = false) {
        Distribution result;
        const std::string type = node.attribute("type").value();
        auto data = node.child(type.c_str());
        if (type == "fixed" && !vector)
            result.from[0] = number(data.attribute("value"));
        else if (type == "fixed_vector" && vector) {
            auto p = point(data);
            result.from = {p.x, p.y, p.z};
        } else if ((type == "linear_random" && !vector) || (type == "linear_random_vector" && vector) ||
                   (type == "box_random" && vector)) {
            result.mode = type == "box_random" ? Distribution::Box : Distribution::Range;
            if (vector) {
                auto a = point(data.child("from")), b = point(data.child("to"));
                result.from = {a.x, a.y, a.z};
                result.to = {b.x, b.y, b.z};
            } else {
                result.from[0] = number(data.child("from").attribute("value"));
                result.to[0] = number(data.child("to").attribute("value"));
            }
        } else
            throw std::runtime_error("Unsupported particle distribution: " + type);
        if (!wide)
            for (unsigned i = 0; i < 3; ++i) {
                result.from[i] = float(result.from[i]);
                result.to[i] = float(result.to[i]);
            }
        return result;
    };
    auto curve = [&](pugi::xml_node node) {
        ParticleCurve result;
        result.loopLeft = node.child("looped").attribute("left").as_bool();
        result.loopRight = node.child("looped").attribute("right").as_bool();
        for (auto key : node.children("key")) {
            ParticleCurve::Key k;
            k.type = key.attribute("type").as_uint();
            k.time = float(number(key.attribute("time")));
            k.in = float(number(key.attribute("in")));
            k.out = float(number(key.attribute("out"), k.in));
            k.inX = float(number(key.attribute("intanx")));
            k.inY = float(number(key.attribute("intany")));
            k.outX = float(number(key.attribute("outtanx")));
            k.outY = float(number(key.attribute("outtany")));
            if (k.type > 4)
                throw std::runtime_error("Unsupported particle curve type");
            result.keys.push_back(k);
        }
        std::stable_sort(result.keys.begin(), result.keys.end(),
                         [](auto a, auto b) { return a.time < b.time; });
        return result;
    };
    auto append = [&](pugi::xml_node xml, Type type, size_t inputs) {
        Node node{};
        node.type = type;
        node.inputs.resize(inputs);
        if (type == Type::Float || type == Type::Vector || type == Type::Distribution ||
            type == Type::Distribution3)
            node.distribution =
                distribution(xml.child("distribution"), type == Type::Vector || type == Type::Distribution3);
        node.collective = xml.child("burstCollective").attribute("value").as_bool();
        if (type == Type::Linear)
            node.speed = float(number(xml.child("speed").attribute("value")));
        if (type == Type::Gravity)
            node.gravity = point(xml.child("gravity"));
        if (type == Type::Track)
            node.curves[0] = curve(xml.child("track"));
        if (type == Type::Track3) {
            node.curves[0] = curve(xml.child("trackX"));
            node.curves[1] = curve(xml.child("trackY"));
            node.curves[2] = curve(xml.child("trackZ"));
        }
        nodes_.push_back(std::move(node));
    };
    for (auto xml : root.child("parameters").children("parameter")) {
        const std::string type = xml.attribute("type").value();
        if (type == "Float")
            append(xml, Type::Float, 1);
        else if (type == "Vector3")
            append(xml, Type::Vector, 1);
        else if (type == "Spawn Global Position")
            append(xml, Type::Spawn, 0);
        else
            throw std::runtime_error("Unsupported particle parameter: " + type);
    }
    parameters_ = modifierBegin_ = nodes_.size();
    for (auto xml : root.child("modifiers").children("modifier")) {
        const std::string type = xml.attribute("type").value();
        if (type == "Linear addition")
            append(xml, Type::Linear, 1);
        else if (type == "Gravity")
            append(xml, Type::Gravity, 1);
        else if (type == "Move")
            append(xml, Type::Move, 2);
        else
            throw std::runtime_error("Unsupported particle modifier: " + type);
    }
    functionBegin_ = nodes_.size();
    const std::map<std::string, std::pair<Type, size_t>> functions{
        {"Distribution", {Type::Distribution, 0}},
        {"Distribution3", {Type::Distribution3, 0}},
        {"Div", {Type::Div, 2}},
        {"Mul", {Type::Mul, 2}},
        {"Add", {Type::Add, 2}},
        {"Sub", {Type::Sub, 2}},
        {"Add (vector)", {Type::Add3, 2}},
        {"Join", {Type::Join, 3}},
        {"Split", {Type::Split, 1}},
        {"InterpTrack", {Type::Track, 1}},
        {"InterpTrack3", {Type::Track3, 1}}};
    for (auto xml : root.child("functions").children("function")) {
        const std::string type = xml.attribute("type").value();
        auto f = functions.find(type);
        if (f == functions.end())
            throw std::runtime_error("Unsupported particle function: " + type);
        append(xml, f->second.first, f->second.second);
    }
    generator_ = nodes_.size();
    append(root.child("generator"), Type::Generator, 0);
    if (auto phase = root.child("generator").child("phase_distribution"))
        phase_ = distribution(phase, false);
    interval_ = distribution(root.child("generator").child("time_distribution"), false, true);
    burst_ = distribution(root.child("generator").child("burst_distribution"), false, true);
    terminator_ = nodes_.size();
    append(root.child("terminator"), Type::Terminator, 2);
    constructor_ = nodes_.size();
    append(root.child("constructor"), Type::Constructor, 5);
    if (std::string(root.child("generator").attribute("type").value()) != "Generator" ||
        std::string(root.child("terminator").attribute("type").value()) != "Terminator: v1 < v2" ||
        std::string(root.child("constructor").attribute("type").value()) != "Constructor")
        throw std::runtime_error("Unsupported particle control block");
    auto constructor = root.child("constructor");
    texture = normalize(constructor.child("texture").attribute("src").value());
    material = normalize(constructor.child("material").attribute("src").value());
    if (material == "materials/quad_tex_add.mat")
        additive = true;
    else if (material != "materials/quad_tex.mat")
        throw std::runtime_error("Unsupported particle material: " + material);
    slicesX = constructor.child("slice_tex").attribute("x").as_uint();
    slicesY = constructor.child("slice_tex").attribute("y").as_uint();
    if (!slicesX || !slicesY || slicesX > 255 || slicesY > 255)
        throw std::runtime_error("Invalid particle texture slicing");
    // Port types: 0 scalar value, 1 vector value, 2 scalar storage, 3 vector storage.
    auto outputKind = [&](const Port& port) {
        if (port.node >= nodes_.size())
            throw std::runtime_error("Particle block index out of range");
        const auto type = nodes_[port.node].type;
        const unsigned outputs =
            type == Type::Split
                ? 3
                : (type == Type::Generator || type == Type::Terminator || type == Type::Constructor ? 0 : 1);
        if (port.output >= outputs)
            throw std::runtime_error("Particle output index out of range");
        if (type == Type::Linear)
            return 2;
        if (type == Type::Move || type == Type::Gravity)
            return 3;
        return type == Type::Vector || type == Type::Spawn || type == Type::Distribution3 ||
                       type == Type::Add3 || type == Type::Join || type == Type::Track3
                   ? 1
                   : 0;
    };
    auto inputKind = [&](Type type, unsigned slot) {
        if (type == Type::Float)
            return 2;
        if (type == Type::Vector)
            return 3;
        if (type == Type::Move || type == Type::Gravity || type == Type::Add3 || type == Type::Split)
            return 1;
        if (type == Type::Constructor && (slot == 0 || slot == 2))
            return 1;
        return 0;
    };
    std::vector<unsigned> writes(nodes_.size());
    for (auto xml : root.child("connections").children("connection")) {
        const auto input = xml.attribute("input_block").as_uint(),
                   slot = xml.attribute("input_index").as_uint();
        Port port{xml.attribute("output_block").as_uint(), xml.attribute("output_index").as_uint(), true};
        if (input >= nodes_.size() || slot >= nodes_[input].inputs.size())
            throw std::runtime_error("Particle input index out of range");
        auto& destination = nodes_[input].inputs[slot];
        if (destination.connected)
            throw std::runtime_error("Duplicate particle input connection");
        if (outputKind(port) != inputKind(nodes_[input].type, slot))
            throw std::runtime_error("Particle port type mismatch");
        destination = port;
        if (input < parameters_) {
            nodes_[port.node].destination = input;
            ++writes[port.node];
        }
    }
    for (size_t i = parameters_; i < nodes_.size(); ++i) {
        for (const auto& input : nodes_[i].inputs)
            if (!input.connected)
                throw std::runtime_error("Unconnected particle input");
        if (i < functionBegin_ && writes[i] != 1)
            throw std::runtime_error("Particle modifier requires one destination");
    }
    std::vector<unsigned> visited(nodes_.size());
    std::function<void(size_t)> visit = [&](size_t i) {
        if (i < parameters_)
            return;
        if (visited[i] == 1)
            throw std::runtime_error("Particle function cycle");
        if (visited[i] == 2)
            return;
        visited[i] = 1;
        for (const auto& input : nodes_[i].inputs)
            if (input.connected)
                visit(input.node);
        visited[i] = 2;
    };
    for (size_t i = parameters_; i < nodes_.size(); ++i)
        visit(i);
}
ParticleEmitter::ParticleEmitter(std::shared_ptr<const ParticleLayout> layout, ParticleRandom& random)
    : layout_(std::move(layout)), random_(&random) {
    restartGenerator();
}
void ParticleEmitter::restartGenerator() {
    interval_ = layout_->interval_.scalar(*random_);
    const auto burst = layout_->burst_.scalar(*random_);
    if (burst < 0 || burst > 100000)
        throw std::runtime_error("Invalid particle burst");
    burst_ = unsigned(burst);
    phase_ = float(layout_->phase_.scalar(*random_)) * interval_;
}
void ParticleEmitter::emit(unsigned count, ModelVector position) {
    if (count > 100000 || particles_.size() + count > 100000)
        throw std::runtime_error("Particle count limit exceeded");
    particles_.insert(particles_.begin(), count, Particle{});
    for (unsigned i = 0; i < count; ++i)
        particles_[i].values.resize(layout_->parameters_);
    for (size_t index = 0; index < layout_->parameters_; ++index) {
        const auto& node = layout_->nodes_[index];
        auto initial = [&] {
            if (node.type == ParticleLayout::Type::Spawn)
                return position;
            if (node.type == ParticleLayout::Type::Vector)
                return node.distribution.vector(*random_);
            return ModelVector{float(node.distribution.scalar(*random_)), 0, 0};
        };
        if (node.collective) {
            const auto value = initial();
            for (unsigned i = 0; i < count; ++i)
                particles_[i].values[index] = value;
        } else
            for (unsigned i = 0; i < count; ++i)
                particles_[i].values[index] = initial();
    }
    // Texture layout is an additional native parameter after user parameters;
    // rand is called twice even for a single tile.
    for (unsigned i = 0; i < count; ++i) {
        particles_[i].tileX = random_->next() % layout_->slicesX;
        particles_[i].tileY = random_->next() % layout_->slicesY;
    }
}
ModelVector ParticleEmitter::evaluate(const ParticleLayout::Port& port, const Particle& particle,
                                      unsigned depth) {
    if (depth > layout_->nodes_.size())
        throw std::runtime_error("Particle evaluation cycle");
    using T = ParticleLayout::Type;
    const auto& node = layout_->nodes_[port.node];
    auto input = [&](unsigned i) { return evaluate(node.inputs.at(i), particle, depth + 1); };
    auto scalar = [](float x) { return ModelVector{x, 0, 0}; };
    switch (node.type) {
    case T::Float:
    case T::Vector:
    case T::Spawn:
        return particle.values.at(port.node);
    case T::Distribution:
        return scalar(float(node.distribution.scalar(*random_)));
    case T::Distribution3:
        return node.distribution.vector(*random_);
    case T::Add: {
        const auto b = input(1).x;
        return scalar(input(0).x + b);
    }
    case T::Sub: {
        const auto a = input(0).x;
        return scalar(a - input(1).x);
    }
    case T::Mul: {
        const auto b = input(1).x;
        return scalar(input(0).x * b);
    }
    case T::Div: {
        const auto a = input(0).x;
        return scalar(a / input(1).x);
    }
    case T::Add3: {
        const auto b = input(1);
        return add(input(0), b);
    }
    case T::Join: {
        const auto x = input(0).x, y = input(1).x, z = input(2).x;
        return {x, y, z};
    }
    case T::Split: {
        const auto v = input(0);
        return scalar(port.output == 0 ? v.x : port.output == 1 ? v.y : v.z);
    }
    case T::Track:
        return scalar(node.curves[0].sample(input(0).x));
    case T::Track3: {
        const auto x = node.curves[0].sample(input(0).x), y = node.curves[1].sample(input(0).x),
                   z = node.curves[2].sample(input(0).x);
        return {x, y, z};
    }
    default:
        throw std::runtime_error("Particle storage/control block used as a value");
    }
}
void ParticleEmitter::tick(double seconds, ModelVector position) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid particle update time");
    const auto& term = layout_->nodes_[layout_->terminator_];
    std::erase_if(particles_, [&](const Particle& p) {
        const auto a = evaluate(term.inputs[0], p).x;
        return evaluate(term.inputs[1], p).x > a;
    });
    if (generating_ && interval_ >= .0001) {
        phase_ += seconds;
        unsigned emissions = 0;
        while (interval_ <= phase_) {
            if (interval_ < .0001)
                break;
            if (++emissions > 100000)
                throw std::runtime_error("Particle emission limit exceeded");
            phase_ -= interval_;
            emit(burst_, position);
            interval_ = layout_->interval_.scalar(*random_);
            const auto burst = layout_->burst_.scalar(*random_);
            if (burst < 0 || burst > 100000)
                throw std::runtime_error("Invalid particle burst");
            burst_ = unsigned(burst);
        }
    }
    using T = ParticleLayout::Type;
    const float dt = float(seconds);
    for (size_t i = layout_->modifierBegin_; i < layout_->functionBegin_; ++i) {
        const auto& modifier = layout_->nodes_[i];
        for (auto& p : particles_) {
            auto value = evaluate(modifier.inputs[0], p);
            if (modifier.type == T::Linear)
                value.x = float(double(value.x) + double(modifier.speed) * dt);
            else if (modifier.type == T::Gravity)
                value = add(value, scale(modifier.gravity, dt));
            else if (modifier.type == T::Move)
                value = add(value, scale(evaluate(modifier.inputs[1], p), dt));
            p.values[modifier.destination] = value;
        }
    }
}
std::vector<ParticleSprite> ParticleEmitter::sprites() {
    const auto& constructor = layout_->nodes_[layout_->constructor_];
    std::vector<ParticleSprite> result;
    result.reserve(particles_.size());
    for (const auto& p : particles_) {
        ParticleSprite s;
        s.size = evaluate(constructor.inputs[1], p).x;
        s.position = evaluate(constructor.inputs[0], p);
        s.tileX = p.tileX;
        s.tileY = p.tileY;
        result.push_back(s);
    }
    for (size_t i = 0; i < particles_.size(); ++i) {
        const auto& p = particles_[i];
        auto color = evaluate(constructor.inputs[2], p);
        result[i].color = {bounded(color.x), bounded(color.y), bounded(color.z)};
        result[i].alpha = bounded(evaluate(constructor.inputs[3], p).x);
    }
    for (size_t i = 0; i < particles_.size(); ++i) {
        const auto& p = particles_[i];
        const auto center = evaluate(constructor.inputs[0], p);
        const auto angle = evaluate(constructor.inputs[4], p).x;
        result[i].rotation = angle;
        const float radians = angle * 3.14159265358979323846f / 180, c = std::cos(radians),
                    s = std::sin(radians), x = result[i].position.x - center.x,
                    y = result[i].position.y - center.y;
        result[i].position.x = center.x + x * c - y * s;
        result[i].position.y = center.y + x * s + y * c;
    }
    return result;
}
ModelVector ParticleEmitter::parameter(size_t particle, size_t index) const {
    return particles_.at(particle).values.at(index);
}
} // namespace deadtime
